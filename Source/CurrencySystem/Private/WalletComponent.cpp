// Copyright (C) Thyke. All Rights Reserved.

#include "WalletComponent.h"
#include "CurrencySubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(WalletComponent)

UWalletComponent::UWalletComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Currency logic is event-based and typically doesn't need per-frame updates.
	PrimaryComponentTick.bCanEverTick = false;
}

void UWalletComponent::BeginPlay()
{
	Super::BeginPlay();
}

FCurrency UWalletComponent::GetBalance(FGameplayTag Currency) const
{
	if (const FCurrency* Found = Balances.Find(Currency))
		return *Found;
	
	// Return a safe 'Zero' object instead of a null pointer or garbage data
	return FCurrency::Zero(Currency);
}

void UWalletComponent::AddCurrency(const FCurrency& Amount)
{
	if (Amount.IsZero()) return;

	// Macro ensures the key exists and validates data integrity before we touch it
	ENSURE_CURRENCY_SLOT_VALIDATED(Balances, Amount.CurrencyTag);

	FCurrency& Slot = Balances[Amount.CurrencyTag];
	const FCurrency Old = Slot;
	
	Slot += Amount;

	// Final integrity check after operation
	ensure(Slot.CurrencyTag == Amount.CurrencyTag);
    
	// Broadcast events to UI / Game Logic
	OnCurrencyAdded.Broadcast(Amount.CurrencyTag, Amount, Slot);
	OnBalanceChanged.Broadcast(Amount.CurrencyTag, Old, Slot);
}

/* ------------------------------------------------------------------------- */
/* Complex Spending Logic with Auto-Conversion                              */
/* ------------------------------------------------------------------------- */
bool UWalletComponent::SpendCurrency(
	const FCurrency& Price, bool bGiveChange,
	TMap<FGameplayTag, FCurrency>& OutChangeMap)
{
	OutChangeMap.Empty();
	
	if (Price.IsZero()) return true; // Free items are always affordable

	// -------------------------------------------------------------------------
	// PHASE 1: Direct Payment Check
	// If the user has enough of the EXACT currency requested, just use that.
	// -------------------------------------------------------------------------
	FCurrency BalanceInCurrency = GetBalance(Price.CurrencyTag);
	if (BalanceInCurrency >= Price)
	{
		ENSURE_CURRENCY_SLOT(Balances, Price.CurrencyTag);
		FCurrency& Slot = Balances[Price.CurrencyTag];
		const FCurrency Old = Slot;
		
		Slot -= Price;

		OnCurrencySpent.Broadcast(Price.CurrencyTag, Price, Slot);
		OnBalanceChanged.Broadcast(Price.CurrencyTag, Old, Slot);
		return true;
	}

	// -------------------------------------------------------------------------
	// PHASE 2: Multi-Currency Payment (Auto-Convert)
	// User doesn't have enough of the target currency. Check if their TOTAL wealth
	// (converted to target currency) is enough.
	// -------------------------------------------------------------------------
	
	UCurrencySubsystem* CS = GetWorld()->GetGameInstance()->GetSubsystem<UCurrencySubsystem>();
	if (!CS) 
	{ 
		// Without the subsystem, we can't calculate exchange rates. Fail safe.
		OnInsufficientFunds.Broadcast(Price); 
		return false; 
	}

	// Step 2a: Calculate Total Wealth in terms of the Price's Currency
	int64 TotalMinorConverted = 0;
	
	// Helper map to track how much we plan to take from each currency slot
	TMap<FGameplayTag, int64> UsageInTargetCurrency;

	for (const auto& Pair : Balances)
	{
		if (Pair.Value.IsZero()) continue;
		
		FCurrency Converted;
		// Try to convert "Wallet Currency" -> "Price Currency"
		if (CS->Convert(Pair.Value, Price.CurrencyTag, Converted))
		{
			TotalMinorConverted += Converted.MinorTotal;
			UsageInTargetCurrency.Add(Pair.Key, 0); // Prepare slot for allocation
		}
	}

	// Check if even the combined wealth is enough
	if (TotalMinorConverted < Price.MinorTotal)
	{
		OnInsufficientFunds.Broadcast(Price);
		return false; 
	}

	// -------------------------------------------------------------------------
	// PHASE 3: Greedy Allocation Strategy
	// Decide WHICH currencies to burn to pay the debt.
	// Priority:
	// 1. The Target Currency itself (drain it first).
	// 2. Other currencies (arbitrary order, usually map iteration order).
	// -------------------------------------------------------------------------
	
	int64 RemainingToPay = Price.MinorTotal;

	// First, drain the matching currency if it exists
	if (Balances.Contains(Price.CurrencyTag))
	{
		const FCurrency& CurrentBalance = Balances[Price.CurrencyTag];
		int64 ToUseFromThis = FMath::Min(CurrentBalance.MinorTotal, RemainingToPay);
		
		UsageInTargetCurrency[Price.CurrencyTag] = ToUseFromThis;
		RemainingToPay -= ToUseFromThis;
	}

	// Then, drain other currencies until debt is paid
	for (const auto& Pair : Balances)
	{
		if (RemainingToPay <= 0) break; // Debt paid
		if (Pair.Key == Price.CurrencyTag) continue; // Already handled
		if (Pair.Value.IsZero()) continue;

		FCurrency ConvertedBalance;
		// We re-convert here to be safe, though optimization is possible
		if (!CS->Convert(Pair.Value, Price.CurrencyTag, ConvertedBalance)) continue;

		int64 ToUseFromThis = FMath::Min(ConvertedBalance.MinorTotal, RemainingToPay);
		
		UsageInTargetCurrency[Pair.Key] = ToUseFromThis;
		RemainingToPay -= ToUseFromThis;
	}

	// -------------------------------------------------------------------------
	// PHASE 4: Execution (Deduct Funds)
	// Now we actually modify the wallet balances based on the plan above.
	// -------------------------------------------------------------------------
	int64 TotalSpentInTargetCurrency = 0;
	
	for (const auto& Entry : UsageInTargetCurrency)
	{
		if (Entry.Value <= 0) continue;

		FGameplayTag CurrencyKey = Entry.Key;
		int64 AmountToSpendInTargetCurrency = Entry.Value;
		
		ENSURE_CURRENCY_SLOT(Balances, CurrencyKey);
		FCurrency& Slot = Balances[CurrencyKey];
		const FCurrency OldBalance = Slot;

		// Calculate how much to remove from the SOURCE currency
		FCurrency AmountToSpendInSourceCurrency;
		
		if (CurrencyKey == Price.CurrencyTag)
		{
			// Direct spending, no conversion needed
			AmountToSpendInSourceCurrency = FCurrency{CurrencyKey, AmountToSpendInTargetCurrency};
		}
		else
		{
			// Reverse Convert: We know we need X amount in Target Currency.
			// How much is that in Source Currency?
			FCurrency TargetCurrencyVal{Price.CurrencyTag, AmountToSpendInTargetCurrency};
			if (!CS->Convert(TargetCurrencyVal, CurrencyKey, AmountToSpendInSourceCurrency))
			{
				continue; // Should not happen given previous checks, but safety first
			}
			
			// Clamp to ensure we don't accidentally overdraw due to rounding errors
			if (AmountToSpendInSourceCurrency.MinorTotal > Slot.MinorTotal)
			{
				AmountToSpendInSourceCurrency.MinorTotal = Slot.MinorTotal;
			}
		}

		// Apply deduction
		Slot.MinorTotal -= AmountToSpendInSourceCurrency.MinorTotal;
		TotalSpentInTargetCurrency += AmountToSpendInTargetCurrency;

		// Notify listeners
		OnCurrencySpent.Broadcast(CurrencyKey, AmountToSpendInSourceCurrency, Slot);
		OnBalanceChanged.Broadcast(CurrencyKey, OldBalance, Slot);
	}

	// -------------------------------------------------------------------------
	// PHASE 5: Give Change (Para Üstü)
	// Because of conversion rates (e.g., spending 1 Gold for a 10 Silver item),
	// we might have "overpaid" in value. Return the difference.
	// -------------------------------------------------------------------------
	if (bGiveChange && TotalSpentInTargetCurrency > Price.MinorTotal)
	{
		int64 ChangeAmount = TotalSpentInTargetCurrency - Price.MinorTotal;
		FCurrency ChangeCurrency{Price.CurrencyTag, ChangeAmount};
		
		// Add the change back to the wallet
		AddCurrency(ChangeCurrency);
		OutChangeMap.Add(Price.CurrencyTag, ChangeCurrency);
		
		// Notify listeners specifically about the change event
		FChangeMapData ChangeData;
		ChangeData.ChangeMap = OutChangeMap;
		ChangeData.NewBalances = Balances;
		OnChangeGiven.Broadcast(ChangeData);
	}

	return true;
}