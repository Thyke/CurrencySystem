// Copyright (C) Thyke. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "CurrencyTableRow.generated.h"

/**
 * FCurrencyTableRow
 * * Structure used to define currency metadata in a Data Table.
 * * USAGE:
 * Create a DataTable asset in the editor and pick this struct as the Row Type.
 * Each row represents a unique currency type in the game (e.g., USD, Gold, Gems).
 * * PURPOSE:
 * Provides central configuration for display names, fractional units, and exchange rates.
 */
USTRUCT(BlueprintType)
struct FCurrencyTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/**
	 * Unique identifier for this currency.
	 * Example: "Currency.USD", "Currency.Fantasy.Gold"
	 * Best Practice: Matches the RowName in the DataTable for consistency.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CurrencyTag;

	/**
	 * The display name for the major unit.
	 * Example: "Dollar", "Lira", "Credit".
	 * Used in UI: "5 Dollars".
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText MajorUnitDisplayName;

	/**
	 * The display name for the minor (fractional) unit.
	 * Example: "Cent", "Kuruş".
	 * Used in UI: "50 Cents".
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText MinorUnitDisplayName;

	/**
	 * Defines how many minor units make up one major unit.
	 * * EXAMPLES:
	 * - USD/TRY: 100 (100 Cents = 1 Dollar)
	 * - JPY (Yen): 1 (No fractional unit usually)
	 * - Ancient Gold: 1 (If not divisible)
	 * * CRITICAL: Must be > 0. Used for splitting logic in Subsystem.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinorPerMajor = 100;

	/**
	 * The Exchange Rate relative to the project's "Reference Currency".
	 * * LOGIC:
	 * Value = (1 Unit of THIS currency) in terms of Reference Currency.
	 * * EXAMPLE (Assuming Reference is USD):
	 * - Row "USD": Rate = 1.0 (Baseline)
	 * - Row "EUR": Rate = 1.1 (1 Euro is worth 1.1 USD)
	 * - Row "TRY": Rate = 0.03 (1 TRY is worth 0.03 USD)
	 * * Used by CurrencySubsystem::Convert() to calculate cross-rates.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	double RateToReference = 1.0;
};