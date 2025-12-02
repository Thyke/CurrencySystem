// Copyright (C) Thyke. All Rights Reserved.


#include "CurrencySubsystem.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(CurrencySubsystem)


const FCurrencyTableRow* UCurrencySubsystem::GetRow(
	FGameplayTag CurrencyTag) const
{
	// Ensure the table is assigned to avoid runtime crashes or silent failures
	return CurrencyTable
		? CurrencyTable->FindRow<FCurrencyTableRow>(
			CurrencyTag.GetTagName(), TEXT("CurrencyLookup"))
		: nullptr;
}

void UCurrencySubsystem::SplitCurrency(
	const FCurrency& Currency, int64& OutMajor, int32& OutMinor) const
{
	if (!CurrencyTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrencySystem: Currency Table is null! Cannot split currency."));
		OutMajor = 0; OutMinor = 0;
		return;
	}

	if (const FCurrencyTableRow* Row = GetRow(Currency.CurrencyTag))
	{
		// Delegate the math to the struct itself using the table's definition
		Currency.ToMajorMinor(OutMajor, OutMinor, Row->MinorPerMajor);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrencySystem: Tag '%s' not found in DataTable!"), *Currency.CurrencyTag.ToString());
		OutMajor = 0; OutMinor = 0;
	}
}

bool UCurrencySubsystem::Convert(
	const FCurrency& InCurrency, FGameplayTag TargetCurrency, FCurrency& OutCurrency) const
{
	// 0. Optimization: If tags are identical, just copy.
	if (InCurrency.CurrencyTag == TargetCurrency) 
	{ 
		OutCurrency = InCurrency; 
		return true; 
	}

	const FCurrencyTableRow* FromRow = GetRow(InCurrency.CurrencyTag);
	const FCurrencyTableRow* ToRow   = GetRow(TargetCurrency);
	
	// Fail gracefully if definitions are missing
	if (!FromRow || !ToRow) return false;

	// Safety check: Prevent Division by Zero if data is bad
	if (FMath::IsNearlyZero(ToRow->RateToReference) || 
		FMath::IsNearlyZero(FromRow->RateToReference))
	{
		UE_LOG(LogTemp, Error, TEXT("CurrencySystem: Invalid Exchange Rate (Zero) detected for %s or %s"), 
			*InCurrency.CurrencyTag.ToString(), *TargetCurrency.ToString());
		return false;
	}
	
	// --- MATH LOGIC START ---
	// We use 'double' for intermediate calculations to preserve precision before rounding back to integer.
	
	// 1) Normalize Source to Reference Currency (Value * SourceRate)
	// Example: 100 Gold (Rate 2.0) -> 200 Reference Units
	const double SourceMinorInReference = static_cast<double>(InCurrency.MinorTotal) * FromRow->RateToReference;
	
	// 2) Convert Reference to Target Currency (Value / TargetRate)
	// Example: 200 Reference Units -> Target Silver (Rate 0.5) -> 400 Silver
	const double TargetMinorValue = SourceMinorInReference / ToRow->RateToReference;
	
	// 3) Apply Minor Unit Scaling 
	// If Source uses 100 units/major and Target uses 1000 units/major, we must multiply by 10.
	const double ScalingFactor = static_cast<double>(ToRow->MinorPerMajor) / static_cast<double>(FromRow->MinorPerMajor);
	const double FinalTargetMinor = TargetMinorValue * ScalingFactor;

	// 4) Overflow Protection & Rounding
	// Check if the result fits into int64 to prevent wrap-around bugs in late-game economy.
	if (FinalTargetMinor > static_cast<double>(TNumericLimits<int64>::Max()) ||
		FinalTargetMinor < static_cast<double>(TNumericLimits<int64>::Min()))
	{
		UE_LOG(LogTemp, Error, TEXT("CurrencySystem: Conversion resulted in Overflow! Value too large."));
		return false; 
	}

	// Finalize: Round to nearest integer (half-to-zero) to determine the exact minor units.
	OutCurrency.CurrencyTag = TargetCurrency;
	OutCurrency.MinorTotal  = static_cast<int64>(FMath::RoundHalfToZero(FinalTargetMinor));
	
	return true;
}