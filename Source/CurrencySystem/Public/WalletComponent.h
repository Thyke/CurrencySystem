// Copyright (C) Thyke. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CurrencyTypes.h"
#include "Components/ActorComponent.h"
#include "WalletComponent.generated.h"

/**
 * FChangeMapData
 * * Wrapper struct for passing TMap<FGameplayTag, FCurrency> to Blueprints via Delegates.
 * * REASONING:
 * Unreal Engine Blueprints do not support TMap as a direct parameter in Dynamic Multicast Delegates.
 * Wrapping it in a USTRUCT allows us to broadcast the "Change Given" (Para Üstü) data cleanly.
 */
USTRUCT(BlueprintType)
struct FChangeMapData
{
	GENERATED_BODY()
	
	/** The "change" (money returned) given back to the player, categorized by currency type. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, FCurrency> ChangeMap;
	
	/** Snapshot of the wallet balances AFTER the transaction is complete. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, FCurrency> NewBalances;
};

// --- Delegates / Events ---
// These allow the UI (Widgets) to update automatically without polling (Event-Driven UI).

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWalletCurrencyChanged, FGameplayTag, Currency, FCurrency, OldBalance, FCurrency, NewBalance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnWalletCurrencyDelta, FGameplayTag, Currency, FCurrency, Delta, FCurrency, NewBalance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWalletChangeGiven, FChangeMapData, ChangeData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWalletInsufficient, FCurrency, Requested);

/**
 * UWalletComponent
 * * A flexible inventory for money that can be attached to any Actor (PlayerState, Controller, NPC).
 * * KEY FEATURES:
 * - Multi-Currency Support: Can hold USD, Gold, Gems simultaneously.
 * - Smart Spending: If the primary currency is insufficient, it can intelligently 
 * convert and spend other available currencies (Auto-Exchange).
 * - Event-Driven: Broadcasts changes to update UI instantly.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CURRENCYSYSTEM_API UWalletComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWalletComponent(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

public:
	/**
	 * The core storage. Maps a Currency Tag (e.g., Currency.Gold) to a monetary value.
	 * NOTE: Direct modification is protected; use AddCurrency/SpendCurrency APIs.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Currency")
	TMap<FGameplayTag, FCurrency> Balances;

	// --- Event Dispatchers ---

	/** Fired whenever ANY balance changes (Gain or Loss). Good for updating text labels. */
	UPROPERTY(BlueprintAssignable, Category="Currency|Events")
	FOnWalletCurrencyChanged OnBalanceChanged;

	/** Fired strictly when money is GAINED. Good for playing "Cha-ching" sounds or popup effects. */
	UPROPERTY(BlueprintAssignable, Category="Currency|Events")
	FOnWalletCurrencyDelta   OnCurrencyAdded;

	/** Fired strictly when money is SPENT. */
	UPROPERTY(BlueprintAssignable, Category="Currency|Events")
	FOnWalletCurrencyDelta   OnCurrencySpent;

	/** Fired when a transaction results in "Change" being returned (e.g., Overpayment conversion). */
	UPROPERTY(BlueprintAssignable, Category="Currency|Events")
	FOnWalletChangeGiven     OnChangeGiven;

	/** Fired when a transaction fails due to lack of funds. Good for "Not Enough Minerals" error sounds. */
	UPROPERTY(BlueprintAssignable, Category="Currency|Events")
	FOnWalletInsufficient    OnInsufficientFunds;

	// --------------------------- Blueprint API ----------------------------------
	
	/** Safely retrieves the balance for a specific currency. Returns Zero if tag doesn't exist. */
	UFUNCTION(BlueprintCallable, Category="Currency")
	FCurrency GetBalance(FGameplayTag Currency) const;

	/** Adds funds to the wallet and broadcasts relevant events. */
	UFUNCTION(BlueprintCallable, Category="Currency")
	void   AddCurrency(const FCurrency& Amount);

	/**
	 * Attempts to pay a specific price.
	 * * LOGIC:
	 * 1. Checks if the specific currency exists and is sufficient.
	 * 2. If NOT, and if the system allows, it attempts to convert other currencies 
	 * to cover the cost (Multi-Currency Transaction).
	 * * @param Price          The amount and type of currency required.
	 * @param bGiveChange    If true, overpayment (due to conversion) is returned as change.
	 * @param OutChangeMap   [Out] Detailed report of any change returned.
	 * @return               True if the transaction was successful.
	 */
	UFUNCTION(BlueprintCallable, Category="Currency")
	bool   SpendCurrency(const FCurrency& Price,
					  bool bGiveChange,
					  /*out*/ TMap<FGameplayTag, FCurrency>& OutChangeMap);
};