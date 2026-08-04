#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AshenPlayerHUDWidget.generated.h"

class UAshenAttributeComponent;
class UBorder;
class UButton;
class UCanvasPanel;
class UProgressBar;
class UTextBlock;
class UVerticalBox;
class UWidget;

UCLASS()
class ASHENKEEP_API UAshenPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		Category = "Ashen Keep|HUD"
	)
	void InitializeWithAttributes(
		UAshenAttributeComponent* InAttributes
	);

	UFUNCTION(
		BlueprintCallable,
		Category = "Ashen Keep|HUD"
	)
	void SetAttributeComponent(
		UAshenAttributeComponent* InAttributes
	);

protected:
	virtual void NativeConstruct() override;

	virtual void NativeTick(
		const FGeometry& MyGeometry,
		float InDeltaTime
	) override;

	virtual FReply NativeOnPreviewMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent
	) override;

	virtual FReply NativeOnKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent
	) override;

private:
	void BuildInterface();
	void BuildStartupMenu(
		UCanvasPanel* RootCanvas
	);

	void ShowStartupMenu();
	void ApplyStartupMenuInput();
	void CloseStartupMenu();

	bool IsScreenPositionInsideWidget(
		const UWidget* Widget,
		const FVector2D& ScreenPosition
	) const;

	void ResolveAttributeComponent();
	void RefreshBars();

	UTextBlock* CreateText(
		FName WidgetName,
		const FString& InText,
		int32 FontSize,
		const FLinearColor& InColor,
		int32 LetterSpacing = 0
	);

	UBorder* CreateBorder(
		FName WidgetName,
		const FLinearColor& InColor,
		const FMargin& InPadding
	);

	UButton* CreateMenuButton(
		FName WidgetName,
		const FString& LabelText
	);

	UWidget* CreateStatRow(
		FName RowName,
		const FString& LabelText,
		UProgressBar*& OutProgressBar,
		const FLinearColor& FillColor
	);

	void AddVerticalChild(
		UVerticalBox* ParentBox,
		UWidget* ChildWidget,
		const FMargin& InPadding = FMargin(0.0f)
	);

	void HandleBeginHuntClicked();
	void HandleControlsClicked();
	void HandleQuitClicked();

	UPROPERTY(Transient)
	TObjectPtr<UAshenAttributeComponent> ObservedAttributes;

	/*
	 * Runtime-only names intentionally differ from the old Designer
	 * variables HealthBar / StaminaBar / ManaBar in WBP_PlayerHUD.
	 */
	UPROPERTY(Transient)
	UProgressBar* RuntimeHealthBar = nullptr;

	UPROPERTY(Transient)
	UProgressBar* RuntimeStaminaBar = nullptr;

	UPROPERTY(Transient)
	UProgressBar* RuntimeBloodBar = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> StartupMenuOverlay;

	UPROPERTY(Transient)
	TObjectPtr<UBorder> StartupControlsPanel;

	UPROPERTY(Transient)
	TObjectPtr<UButton> StartupBeginButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> StartupControlsButton;

	UPROPERTY(Transient)
	TObjectPtr<UButton> StartupQuitButton;

	bool bInterfaceBuilt = false;
	bool bStartupMenuOpen = false;
	bool bMenuInputApplied = false;
};
