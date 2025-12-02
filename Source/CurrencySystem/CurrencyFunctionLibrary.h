// Copyright (C) Thyke. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CurrencyTypes.h"
#include "WalletComponent.h"
#include "CurrencyFunctionLibrary.generated.h"

/**
 * UCurrencyFunctionLibrary
 * * A static helper library for common financial operations in the game.
 * * PURPOSE:
 * Encapsulates complex logic like safe transfers, affordability checks, and 
 * wealth consolidation to keep the core Component classes clean.
 * * BEST PRACTICE:
 * Use these functions in Blueprints or C++ for any interaction between two different wallets
 * (e.g., Player paying a Shop, Thief stealing from Player, Bank transfers).
 */
UCLASS()
class CURRENCYSYSTEM_API UCurrencyFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Safely transfers a specific amount from one wallet to another.
	 * * TRANSACTIONAL LOGIC:
	 * 1. Verifies Source has enough funds (direct or via auto-conversion).
	 * 2. Deducts from Source (Spending).
	 * 3. Adds to Target (Deposit).
	 * * FAIL-SAFE:
	 * If deduction fails, no money is added to Target. Money is never created out of thin air.
	 * * @param SourceWallet  The wallet paying the money (e.g., Player).
	 * @param TargetWallet  The wallet receiving the money (e.g., Shop Register).
	 * @param Amount        The amount and currency type to transfer.
	 * @return              True if the transfer was successful.
	 */
	UFUNCTION(BlueprintCallable, Category="Currency|Operations")
	static bool TransferFunds(UWalletComponent* SourceWallet, UWalletComponent* TargetWallet, FCurrency Amount);

	/**
	 * Moves ALL contents of the Source wallet to the Target wallet.
	 * * USE CASE:
	 * "Take All" button when looting a dead body or emptying a cash register at the end of the day.
	 * * BEHAVIOR:
	 * Iterates through every currency in Source, adds it to Target, and clears Source.
	 * * @param SourceWallet  The wallet to empty.
	 * @param TargetWallet  The wallet to fill.
	 * @return              True if any funds were transferred.
	 */
	UFUNCTION(BlueprintCallable, Category="Currency|Operations")
	static bool WithdrawAllFunds(UWalletComponent* SourceWallet, UWalletComponent* TargetWallet);

	/**
	 * Checks if a wallet can afford a specific price, considering ALL available currencies.
	 * * CONTEXT:
	 * Sometimes we just want to grey out a UI button (Purchase) without actually spending money.
	 * This function simulates the conversion logic to see if the user has enough "Total Wealth".
	 * * @param Wallet   The wallet to check.
	 * @param Price    The cost to check against.
	 * @return         True if the wallet has enough value to pay.
	 */
	UFUNCTION(BlueprintCallable, Category="Currency|Analysis", meta=(WorldContext="WorldContextObject"))
	static bool CanAfford(const UObject* WorldContextObject, const UWalletComponent* Wallet, FCurrency Price);

	/**
	 * Calculates the total value of a wallet converted to a single base currency.
	 * * USE CASE:
	 * Displaying "Net Worth" or "Total Gold Equivalent" in the UI.
	 * * @param Wallet          The wallet to analyze.
	 * @param TargetCurrency  The currency to convert everything into (e.g., USD).
	 * @return                The total value in the requested currency.
	 */
	UFUNCTION(BlueprintCallable, Category="Currency|Analysis", meta=(WorldContext="WorldContextObject"))
	static FCurrency GetTotalWealthInCurrency(const UObject* WorldContextObject, const UWalletComponent* Wallet, FGameplayTag TargetCurrency);
};