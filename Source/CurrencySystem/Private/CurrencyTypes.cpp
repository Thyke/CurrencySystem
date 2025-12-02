// Copyright (C) Thyke. All Rights Reserved.

#include "CurrencyTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CurrencyTypes)

FCurrency FCurrency::FromMajorMinor(
    FGameplayTag InCurrency, int64 Major, int32 Minor, int32 MinorPerMajor)
{
    // Ensure the divisor is valid to prevent division by zero errors logic later on
    check(MinorPerMajor > 0);

    // Clamp the minor input to ensure it doesn't exceed the unit definition.
    // Example: Passing 150 cents when 100 cents = 1 dollar.
    // Ideally, the UI should handle this, but we clamp for safety: [0, 99] for a rate of 100.
    const int32 ClampedMinor = FMath::Clamp(Minor, 0, MinorPerMajor - 1);

    // Calculate total minor units
    const int64 Total = Major * MinorPerMajor + ClampedMinor;
    
    return { InCurrency, Total };
}

void FCurrency::ToMajorMinor(
    int64& OutMajor, int32& OutMinor, int32 MinorPerMajor) const
{
    if (MinorPerMajor == 0)
    {
        // Fallback to prevent crash, though logic dictates this shouldn't happen with valid rows.
        OutMajor = 0;
        OutMinor = 0;
        return; 
    }

    // Integer division for the major part
    OutMajor = MinorTotal / MinorPerMajor;
    
    // Modulo for the remaining minor part
    OutMinor = MinorTotal % MinorPerMajor;
}

// -----------------------------------------------------------------------------
// Arithmetic Operator Implementations
// -----------------------------------------------------------------------------

FCurrency FCurrency::operator+(const FCurrency& Other) const
{
    // It makes no sense to add USD to Gold directly. Convert first if needed.
    check(HasSameCurrency(Other));
    return { CurrencyTag, MinorTotal + Other.MinorTotal };
}

FCurrency FCurrency::operator-(const FCurrency& Other) const
{
    check(HasSameCurrency(Other));
    return { CurrencyTag, MinorTotal - Other.MinorTotal };
}

FCurrency& FCurrency::operator+=(const FCurrency& Other)
{
    check(HasSameCurrency(Other));
    MinorTotal += Other.MinorTotal;
    return *this;
}

FCurrency& FCurrency::operator-=(const FCurrency& Other)
{
    check(HasSameCurrency(Other));
    MinorTotal -= Other.MinorTotal;
    return *this;
}

// -----------------------------------------------------------------------------
// Comparison Operator Implementations
// -----------------------------------------------------------------------------

bool FCurrency::operator>=(const FCurrency& Other) const
{
    check(HasSameCurrency(Other));
    return MinorTotal >= Other.MinorTotal;
}

bool FCurrency::operator<(const FCurrency& Other) const
{
    check(HasSameCurrency(Other));
    return MinorTotal < Other.MinorTotal;
}

bool FCurrency::operator>(const FCurrency& Other) const
{
    check(HasSameCurrency(Other));
    return MinorTotal > Other.MinorTotal;
}

bool FCurrency::operator<=(const FCurrency& Other) const
{
    check(HasSameCurrency(Other));
    return MinorTotal <= Other.MinorTotal;
}

bool FCurrency::operator==(const FCurrency& Other) const
{
    check(HasSameCurrency(Other));
    return MinorTotal == Other.MinorTotal;
}

bool FCurrency::operator!=(const FCurrency& Other) const
{
    check(HasSameCurrency(Other));
    return MinorTotal != Other.MinorTotal;
}