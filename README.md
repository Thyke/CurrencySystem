
# Currency System / Para Birimi Sistemi

*Read this in other languages: [English](https://www.google.com/search?q=%23currency-system-english), [Turkish](https://www.google.com/search?q=%23para-birimi-sistemi-t%C3%BCrk%C3%A7e)*

## Currency System (English)

This system provides a robust, integer-based financial management solution for Unreal Engine. It is designed to handle multi-currency wallets, automatic currency conversion, and safe transactions without floating-point errors. It is suitable for RPGs, simulation games, and trading systems.

### Features

  - **Integer-Based Precision:** Uses `int64` for all internal storage to prevent floating-point errors (e.g., 10.50 is stored as 1050).
  - **Multi-Currency Wallets:** A single component (`UWalletComponent`) can store multiple currency types (Gold, Silver, USD, Gems) simultaneously.
  - **Auto-Conversion Spending:** If a wallet lacks specific funds, the system can automatically convert and spend other available currencies based on defined exchange rates ("Greedy" allocation).
  - **Data-Driven Architecture:** Currencies and exchange rates are defined in a `DataTable` using `FGameplayTags`.
  - **Event-Driven UI:** Delegates for balance changes, additions, and insufficient funds make UI integration seamless.
  - **Safe Transaction Library:** Static helper library for secure fund transfers between entities (e.g., Player to Shop).

### Installation

1.  Copy the `CurrencySystem` folder to your project's "Source" directory.
2.  Add the module dependency to your project's `.Build.cs` file:
    ```csharp
    PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "GameplayTags", "CurrencySystem" });
    ```
3.  Rebuild your project.
4.  **Configuration:**
      - Create a `DataTable` in the editor using the `FCurrencyTableRow` structure.
      - Define your currencies (e.g., `Currency.USD`, `Currency.Gold`) and their `RateToReference`.
      - Assign this DataTable to the `UCurrencySubsystem` (You can create a Blueprint class inheriting from `UGameInstanceSubsystem` or set it in Project Settings if exposed).

### Usage

#### Basic Usage (Wallet Component)

Attach `UWalletComponent` to your `PlayerState` or `Character`.

```cpp
// In your Character or PlayerState
void AMyCharacter::AddGold(int32 Amount)
{
    // Construct the currency amount (e.g., 500 Gold)
    FCurrency GoldAmount = FCurrency::FromMajorMinor(
        FGameplayTag::RequestGameplayTag("Currency.Gold"), 
        Amount, 
        0, 
        1 // MinorPerMajor (if Gold has no fractional unit)
    );

    // Add to wallet
    WalletComponent->AddCurrency(GoldAmount);
}
```

#### Advanced Spending (Auto-Conversion)

The system can automatically convert other currencies if the primary one is insufficient.

```cpp
void AMyCharacter::BuyItem(FCurrency Price)
{
    TMap<FGameplayTag, FCurrency> OutChange;

    // Try to spend. If "Currency.Gold" is missing, it will try to use "Currency.Silver" 
    // based on the exchange rates in the Subsystem.
    bool bSuccess = WalletComponent->SpendCurrency(
        Price, 
        true,       // bGiveChange: Return overpayment as change
        OutChange   // Output: Details of change given
    );

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Item purchased!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Insufficient funds (even after conversion)."));
    }
}
```

#### Helper Library (Safe Transfers)

Use `CurrencyFunctionLibrary` for interactions between two wallets (e.g., Shop System).

```cpp
void AShop::SellItemToPlayer(UWalletComponent* PlayerWallet, UWalletComponent* ShopRegister, FCurrency Price)
{
    // Checks affordability (including conversion) without spending yet
    if (UCurrencyFunctionLibrary::CanAfford(this, PlayerWallet, Price))
    {
        // Safely deducts from Player and adds to Shop Register
        if (UCurrencyFunctionLibrary::TransferFunds(PlayerWallet, ShopRegister, Price))
        {
            GiveItemToPlayer();
        }
    }
}
```

### Technical Details

#### Floating Point Safety

To avoid precision loss common in game economies, this system stores values as `int64 MinorTotal`.

  - **Example:** If `Currency.USD` has `MinorPerMajor = 100`, then **$10.55** is stored internally as **1055**.
  - All math operations are performed on integers. Floating-point logic is only used transiently during exchange rate conversion in the Subsystem.

#### The "Greedy" Spending Algorithm

When `SpendCurrency` is called:

1.  It checks the specific currency balance first.
2.  If insufficient, it calculates the **Total Wealth** of the wallet in the target currency.
3.  If affordable, it burns the target currency first, then iteratively converts and burns other currencies until the debt is paid.
4.  If the conversion results in overpayment, the difference is calculated and returned as "Change".

-----

## Para Birimi Sistemi (Türkçe)

Bu sistem, Unreal Engine için sağlam, tam sayı (integer) tabanlı bir finansal yönetim çözümü sunar. Çoklu para birimi cüzdanlarını, otomatik kur dönüşümünü ve küsurat hatası olmayan güvenli işlemleri yönetmek için tasarlanmıştır. RPG, simülasyon ve ticaret sistemleri için uygundur.

### Özellikler

  - **Tam Sayı Hassasiyeti:** Kayan nokta (float) hatalarını önlemek için dahili depolamada `int64` kullanır (örn. 10.50 değeri 1050 olarak saklanır).
  - **Çoklu Para Birimi Cüzdanı:** Tek bir bileşen (`UWalletComponent`) aynı anda birden fazla para birimini (Altın, Gümüş, Dolar, Elmas vb.) saklayabilir.
  - **Otomatik Dönüştürmeli Harcama:** Cüzdanda istenen para birimi eksikse, sistem tanımlı kurlara göre diğer mevcut para birimlerini otomatik olarak bozdurup harcayabilir.
  - **Veri Odaklı Mimari:** Para birimleri ve döviz kurları `DataTable` ve `FGameplayTags` kullanılarak tanımlanır.
  - **Olay (Event) Tabanlı UI:** Bakiye değişiklikleri, para girişi ve yetersiz bakiye durumları için Delegate'ler sayesinde arayüz güncellemesi kolaydır.
  - **Güvenli İşlem Kütüphanesi:** Varlıklar arası (örn. Oyuncu -\> Dükkan) güvenli para transferi için statik yardımcı kütüphane içerir.

### Kurulum

1.  `CurrencySystem` klasörünü projenizin "Source" dizinine kopyalayın.
2.  Projenizin `.Build.cs` dosyasına modül bağımlılığını ekleyin:
    ```csharp
    PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "GameplayTags", "CurrencySystem" });
    ```
3.  Projenizi yeniden derleyin (Rebuild).
4.  **Konfigürasyon:**
      - Editörde `FCurrencyTableRow` yapısını kullanan bir `DataTable` oluşturun.
      - Para birimlerinizi (örn. `Currency.TL`, `Currency.Gold`) ve `RateToReference` (Referans Kur) değerlerini tanımlayın.
      - Bu tabloyu `UCurrencySubsystem`'e atayın.

### Kullanım

#### Temel Kullanım (Cüzdan Bileşeni)

`UWalletComponent` bileşenini `PlayerState` veya karakterinize ekleyin.

```cpp
// Karakter veya PlayerState içinde
void AMyCharacter::AddGold(int32 Amount)
{
    // Para miktarını oluştur (Örn: 500 Altın)
    FCurrency GoldAmount = FCurrency::FromMajorMinor(
        FGameplayTag::RequestGameplayTag("Currency.Gold"), 
        Amount, 
        0, 
        1 // MinorPerMajor (Altının alt birimi yoksa 1)
    );

    // Cüzdana ekle
    WalletComponent->AddCurrency(GoldAmount);
}
```

#### Gelişmiş Harcama (Otomatik Dönüştürme)

Sistem, birincil para birimi yetersizse diğerlerini otomatik olarak dönüştürebilir.

```cpp
void AMyCharacter::BuyItem(FCurrency Price)
{
    TMap<FGameplayTag, FCurrency> OutChange;

    // Harcama yapmayı dene. Eğer "Currency.Gold" yoksa, Subsystem'deki kurlara göre 
    // otomatik olarak "Currency.Silver" (veya diğerlerini) kullanmaya çalışır.
    bool bSuccess = WalletComponent->SpendCurrency(
        Price, 
        true,       // bGiveChange: Fazla ödeme olursa para üstü ver
        OutChange   // Çıktı: Verilen para üstü detayları
    );

    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Eşya satın alındı!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Yetersiz bakiye (dönüştürme dahil)."));
    }
}
```

#### Yardımcı Kütüphane (Güvenli Transferler)

İki cüzdan arasındaki etkileşimler için `CurrencyFunctionLibrary` kullanın (Örn: Dükkan Sistemi).

```cpp
void AShop::SellItemToPlayer(UWalletComponent* PlayerWallet, UWalletComponent* ShopRegister, FCurrency Price)
{
    // Henüz harcama yapmadan alım gücünü (dönüşümler dahil) kontrol et
    if (UCurrencyFunctionLibrary::CanAfford(this, PlayerWallet, Price))
    {
        // Oyuncudan güvenli şekilde düş ve Dükkan Kasasına ekle
        if (UCurrencyFunctionLibrary::TransferFunds(PlayerWallet, ShopRegister, Price))
        {
            GiveItemToPlayer();
        }
    }
}
```

### Teknik Detaylar

#### Kayan Nokta (Floating Point) Güvenliği

Oyun ekonomilerinde sıkça görülen hassasiyet kayıplarını önlemek için bu sistem değerleri `int64 MinorTotal` olarak saklar.

  - **Örnek:** Eğer `Currency.USD` için `MinorPerMajor = 100` ise, **$10.55** dahili olarak **1055** tam sayısı olarak saklanır.
  - Tüm matematiksel işlemler tam sayılar üzerinde yapılır. Kayan nokta (float/double) mantığı yalnızca Subsystem içindeki kur hesaplamaları sırasında geçici olarak kullanılır.

#### "Açgözlü" (Greedy) Harcama Algoritması

`SpendCurrency` çağrıldığında:

1.  Önce istenen para biriminin bakiyesini kontrol eder.
2.  Yetersizse, cüzdanın **Toplam Varlığını** (hedef para birimine çevirerek) hesaplar.
3.  Eğer güç yetiyorsa; önce hedef para birimini tüketir, ardından borç kapanana kadar diğer para birimlerini sırayla bozdurur.
4.  Dönüştürme işlemi fazla ödemeye neden olursa, aradaki fark hesaplanır ve "Para Üstü" (Change) olarak iade edilir.