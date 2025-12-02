// Copyright (C) Thyke. All Rights Reserved.

#include "CurrencyFunctionLibrary.h"
#include "CurrencySubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CurrencyFunctionLibrary)

bool UCurrencyFunctionLibrary::TransferFunds(UWalletComponent* SourceWallet, UWalletComponent* TargetWallet, FCurrency Amount)
{
	// 1. Validation Checks
	if (!SourceWallet || !TargetWallet)
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrencyFunctionLibrary: Transfer failed. Source or Target wallet is null."));
		return false;
	}

	if (Amount.IsZero())
	{
		// Transferring zero is technically a success (no-op), but we return false to indicate no action taken.
		return false;
	}

	// Self-transfer check (Prevent weird side effects)
	if (SourceWallet == TargetWallet)
	{
		return true; 
	}

	// 2. Attempt to Deduct from Source
	// We don't ask for change (bGiveChange=false) because this is a direct transfer.
	// We assume the user wants to move exactly 'Amount'.
	TMap<FGameplayTag, FCurrency> DummyChange;
	bool bSuccess = SourceWallet->SpendCurrency(Amount, false, DummyChange);

	if (bSuccess)
	{
		// 3. Deposit into Target
		// Critical: This MUST happen if SpendCurrency succeeded. 
		TargetWallet->AddCurrency(Amount);
		return true;
	}
	else
	{
		// Transfer failed (Insufficient funds)
		return false;
	}
}

bool UCurrencyFunctionLibrary::WithdrawAllFunds(UWalletComponent* SourceWallet, UWalletComponent* TargetWallet)
{
	if (!SourceWallet || !TargetWallet) return false;
	
	// Use a temporary buffer to store what we take.
	// We shouldn't modify the map while iterating over it directly if we plan to clear it.
	TMap<FGameplayTag, FCurrency> LootedFunds = SourceWallet->Balances;

	// Optimization: If empty, do nothing.
	if (LootedFunds.Num() == 0) return false;

	bool bTransferredAny = false;

	for (const auto& Pair : LootedFunds)
	{
		const FCurrency& Cash = Pair.Value;
		if (Cash.IsZero()) continue;

		// Move to target
		TargetWallet->AddCurrency(Cash);
		bTransferredAny = true;
	}

	// Clear the source wallet completely
	SourceWallet->Balances.Empty();
	
	// Force update UI for the source wallet (since we bypassed SpendCurrency)
	// We iterate keys to broadcast changes for listeners.
	for (const auto& Pair : LootedFunds)
	{
		SourceWallet->OnBalanceChanged.Broadcast(Pair.Key, Pair.Value, FCurrency::Zero(Pair.Key));
	}

	return bTransferredAny;
}

bool UCurrencyFunctionLibrary::CanAfford(const UObject* WorldContextObject, const UWalletComponent* Wallet, FCurrency Price)
{
	if (!Wallet || !WorldContextObject) return false;
	if (Price.IsZero()) return true;

	// 1. Direct Check (Fast Path)
	FCurrency Balance = Wallet->GetBalance(Price.CurrencyTag);
	if (Balance >= Price) return true;

	// 2. Total Wealth Check (Slow Path - requires conversion)
	// We reuse the logic from SpendCurrency but without modifying state.
	
	const UCurrencySubsystem* CS = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UCurrencySubsystem>();
	if (!CS) return false;

	int64 TotalMinorAvailable = 0;

	for (const auto& Pair : Wallet->Balances)
	{
		if (Pair.Value.IsZero()) continue;

		// Convert everything to the Price's currency
		FCurrency Converted;
		if (CS->Convert(Pair.Value, Price.CurrencyTag, Converted))
		{
			TotalMinorAvailable += Converted.MinorTotal;
		}

		// Optimization: Early exit if we already have enough
		if (TotalMinorAvailable >= Price.MinorTotal) return true;
	}

	return TotalMinorAvailable >= Price.MinorTotal;
}

FCurrency UCurrencyFunctionLibrary::GetTotalWealthInCurrency(const UObject* WorldContextObject, const UWalletComponent* Wallet, FGameplayTag TargetCurrency)
{
	FCurrency TotalResult = FCurrency::Zero(TargetCurrency);
	
	if (!Wallet || !WorldContextObject) return TotalResult;

	const UCurrencySubsystem* CS = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UCurrencySubsystem>();
	if (!CS) return TotalResult;

	for (const auto& Pair : Wallet->Balances)
	{
		const FCurrency& WalletFunds = Pair.Value;
		if (WalletFunds.IsZero()) continue;

		FCurrency ConvertedVal;
		if (CS->Convert(WalletFunds, TargetCurrency, ConvertedVal))
		{
			TotalResult += ConvertedVal;
		}
	}

	return TotalResult;
}