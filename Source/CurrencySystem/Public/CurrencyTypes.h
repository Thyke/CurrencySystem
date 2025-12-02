// Copyright (C) Thyke. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CurrencyTypes.generated.h"

/**
 * Macro to ensure a currency entry exists in a TMap.
 * If the tag is missing, it initializes the entry with Zero balance.
 * Useful before attempting modification operations.
 *
 * @param Map The TMap<FGameplayTag, FCurrency> to check.
 * @param Tag The FGameplayTag representing the currency type.
 */
#define ENSURE_CURRENCY_SLOT(Map, Tag) \
if (!Map.Contains(Tag)) { Map.Add(Tag, FCurrency::Zero(Tag)); }

/**
 * Macro to ensure existence AND validate data integrity of a currency entry.
 * It checks if the Key in the map matches the CurrencyTag inside the Value.
 * If a mismatch occurs (data corruption), it logs an error and resets the value to Zero.
 *
 * @param Map The TMap<FGameplayTag, FCurrency> to validate.
 * @param Tag The FGameplayTag to look up.
 */
#define ENSURE_CURRENCY_SLOT_VALIDATED(Map, Tag) \
if (!Map.Contains(Tag)) { \
    Map.Add(Tag, FCurrency::Zero(Tag)); \
} else { \
    /* Validate existing entry integrity */ \
    ensure(Map[Tag].CurrencyTag == Tag); \
    if (Map[Tag].CurrencyTag != Tag) { \
        UE_LOG(LogTemp, Error, TEXT("Currency mismatch detected! Map Key: %s, Value Tag: %s"), \
            *Tag.ToString(), *Map[Tag].CurrencyTag.ToString()); \
        Map[Tag] = FCurrency::Zero(Tag); /* Fix the corrupted entry */ \
    } \
}

/**
 * FCurrency
 * * Represents a monetary value within the game economy.
 * * CORE PRINCIPLE:
 * To avoid floating-point precision errors common in financial calculations,
 * this struct stores the value as a 64-bit integer (`MinorTotal`) representing 
 * the smallest unit of the currency (e.g., Cents, Kuruş, Penny).
 * * Example: 
 * - 10.50 USD is stored as 1050 (MinorTotal).
 * - 1.00 TRY is stored as 100 (if 1 TRY = 100 Kuruş).
 */
USTRUCT(BlueprintType)
struct FCurrency
{
    GENERATED_BODY()

    /** * The type of currency (e.g., Currency.USD, Currency.Gold). 
     * Used for compatibility checks during arithmetic operations.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Currency")
    FGameplayTag CurrencyTag;

    /** * The total value stored in the minor unit.
     * Direct manipulation is allowed but should be done carefully.
     * Preferred way is using helper functions like FromMajorMinor.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Currency")
    int64 MinorTotal = 0;
    
    /** * Creates a zero-value currency instance.
     * @param InCurrency The tag for the currency type.
     */
    static FCurrency Zero(FGameplayTag InCurrency) { return { InCurrency, 0 }; }

    /**
     * Factory method to create an FCurrency from Major and Minor units.
     * Handles the conversion: (Major * MinorPerMajor) + Minor.
     * * @param InCurrency     The currency type tag.
     * @param Major          The major unit amount (e.g., Dollars).
     * @param Minor          The minor unit amount (e.g., Cents).
     * @param MinorPerMajor  Conversion rate (e.g., 100 for USD/Cents).
     */
    static FCurrency FromMajorMinor(FGameplayTag InCurrency,
        int64 Major, int32 Minor, int32 MinorPerMajor);
    
    /**
     * Decomposes the stored total into Major and Minor components.
     * * @param OutMajor       [Out] The calculated major units.
     * @param OutMinor       [Out] The calculated remainder minor units.
     * @param MinorPerMajor  The conversion rate to use for calculation.
     */
    void ToMajorMinor(int64& OutMajor, int32& OutMinor,
        int32 MinorPerMajor) const;

    /** Check if the balance is exactly zero. */
    bool IsZero() const             { return MinorTotal == 0; }

    /** * Checks if two FCurrency instances share the same currency tag.
     * Arithmetic operations usually require this to be true.
     */
    bool HasSameCurrency(const FCurrency& Other) const
    { return CurrencyTag == Other.CurrencyTag; }

    // --- Operator Overloads ---
    // All arithmetic operators include assertions (check()) to ensure 
    // operations are only performed between identical CurrencyTags.

    FCurrency  operator+(const FCurrency& Other) const;
    FCurrency  operator-(const FCurrency& Other) const;
    FCurrency& operator+=(const FCurrency& Other);
    FCurrency& operator-=(const FCurrency& Other);
    
    // Comparison Operators
    bool    operator>=(const FCurrency& Other) const;
    bool    operator<(const FCurrency& Other) const;
    bool    operator>(const FCurrency& Other) const;
    bool    operator<=(const FCurrency& Other) const;
    bool    operator==(const FCurrency& Other) const;
    bool    operator!=(const FCurrency& Other) const;
};