// Copyright (C) Thyke. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CurrencyTableRow.h"
#include "CurrencyTypes.h"
#include "CurrencySubsystem.generated.h"

/**
 * UCurrencySubsystem
 * * The central manager for currency logic and exchange rates.
 * * ARCHITECTURE NOTE:
 * Inherits from UGameInstanceSubsystem to ensure:
 * 1. Persistence: It survives level transitions (loading screens).
 * 2. Accessibility: Can be accessed globally via GetGameInstance()->GetSubsystem().
 * * RESPONSIBILITIES:
 * - Loading currency definitions from a DataTable.
 * - Handling "Floating Point -> Integer" conversion logic securely.
 * - Calculating exchange rates between arbitrary currencies.
 */
UCLASS()
class CURRENCYSYSTEM_API UCurrencySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	/** * The source of truth for currency definitions. 
	 * Must populate this in the Editor (Project Settings or Blueprint) 
	 * with a DataTable using FCurrencyTableRow structure.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Currency")
	UDataTable* CurrencyTable = nullptr;
	
	/**
	 * Helper to retrieve raw data for a specific currency tag.
	 * Returns nullptr if the table is missing or tag is invalid.
	 */
	const FCurrencyTableRow* GetRow(FGameplayTag CurrencyTag) const;
	
	/**
	 * Decomposes a raw currency value into UI-friendly Major/Minor components.
	 * Example: 150 Minor units -> 1 Major, 50 Minor (if Rate is 100).
	 * * @param Currency  The input currency value.
	 * @param OutMajor  [Out] The whole number part.
	 * @param OutMinor  [Out] The remainder part.
	 */
	UFUNCTION(BlueprintCallable, Category="Currency")
	void SplitCurrency(const FCurrency& Currency,
		int64& OutMajor, int32& OutMinor) const;
	
	/**
	 * Converts an amount from one currency to another using the Reference Rate.
	 * * LOGIC:
	 * Input -> Reference Currency -> Target Currency.
	 * Handles minor unit scaling (e.g., converting from a 1/100 system to a 1/1000 system).
	 * * @param InCurrency      The source amount and type.
	 * @param TargetCurrency  The desired target currency tag.
	 * @param OutCurrency     [Out] The result of the conversion.
	 * @return true if conversion succeeded (valid rows, no overflow, non-zero rates).
	 */
	UFUNCTION(BlueprintCallable, Category="Currency")
	bool Convert(const FCurrency& InCurrency, FGameplayTag TargetCurrency,
		FCurrency& OutCurrency) const;
};