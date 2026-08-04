#include "AshenPlayerHUDWidget.h"

#include "AshenAttributeComponent.h"
#include "AshenPlayerCharacter.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "InputCoreTypes.h"

namespace AshenHUD
{
	const FLinearColor PanelColor(
		0.018f,
		0.012f,
		0.018f,
		0.92f
	);

	const FLinearColor SoftPanelColor(
		0.025f,
		0.018f,
		0.026f,
		0.86f
	);

	const FLinearColor AccentColor(
		0.34f,
		0.045f,
		0.055f,
		0.95f
	);

	const FLinearColor GoldColor(
		0.72f,
		0.52f,
		0.24f,
		1.0f
	);

	const FLinearColor MainTextColor(
		0.90f,
		0.86f,
		0.80f,
		1.0f
	);

	const FLinearColor MutedTextColor(
		0.53f,
		0.49f,
		0.45f,
		1.0f
	);

	const FLinearColor HealthColor(
		0.58f,
		0.025f,
		0.035f,
		1.0f
	);

	const FLinearColor StaminaColor(
		0.66f,
		0.47f,
		0.12f,
		1.0f
	);

	const FLinearColor BloodColor(
		0.31f,
		0.025f,
		0.16f,
		1.0f
	);

	const FLinearColor MenuBackgroundColor(
		0.003f,
		0.002f,
		0.005f,
		0.93f
	);

	const FLinearColor MenuPanelColor(
		0.025f,
		0.015f,
		0.022f,
		0.98f
	);

	const FLinearColor MenuButtonColor(
		0.16f,
		0.025f,
		0.035f,
		1.0f
	);

	float SafePercent(
		const float Current,
		const float Maximum
	)
	{
		if (Maximum <= KINDA_SMALL_NUMBER)
		{
			return 0.0f;
		}

		return FMath::Clamp(
			Current / Maximum,
			0.0f,
			1.0f
		);
	}
}

void UAshenPlayerHUDWidget::
	InitializeWithAttributes(
		UAshenAttributeComponent* InAttributes
	)
{
	ObservedAttributes = InAttributes;
	RefreshBars();
}

void UAshenPlayerHUDWidget::
	SetAttributeComponent(
		UAshenAttributeComponent* InAttributes
	)
{
	InitializeWithAttributes(InAttributes);
}

void UAshenPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	SetIsEnabled(true);
	SetVisibility(ESlateVisibility::Visible);

	ResolveAttributeComponent();
	BuildInterface();
	RefreshBars();
	ShowStartupMenu();
}

void UAshenPlayerHUDWidget::NativeTick(
	const FGeometry& MyGeometry,
	const float InDeltaTime
)
{
	Super::NativeTick(
		MyGeometry,
		InDeltaTime
	);

	if (!ObservedAttributes)
	{
		ResolveAttributeComponent();
	}

	if (bStartupMenuOpen &&
		!bMenuInputApplied)
	{
		ApplyStartupMenuInput();
	}

	RefreshBars();
}

FReply UAshenPlayerHUDWidget::
	NativeOnPreviewMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent
	)
{
	if (bStartupMenuOpen &&
		InMouseEvent.GetEffectingButton() ==
			EKeys::LeftMouseButton)
	{
		const FVector2D ScreenPosition =
			InMouseEvent.GetScreenSpacePosition();

		if (IsScreenPositionInsideWidget(
			StartupBeginButton,
			ScreenPosition
		))
		{
			HandleBeginHuntClicked();
			return FReply::Handled();
		}

		if (IsScreenPositionInsideWidget(
			StartupControlsButton,
			ScreenPosition
		))
		{
			HandleControlsClicked();
			return FReply::Handled();
		}

		if (IsScreenPositionInsideWidget(
			StartupQuitButton,
			ScreenPosition
		))
		{
			HandleQuitClicked();
			return FReply::Handled();
		}

		/*
		 * The dark menu overlay consumes clicks outside the buttons,
		 * so gameplay never receives an accidental attack while the
		 * menu is open.
		 */
		return FReply::Handled();
	}

	return Super::NativeOnPreviewMouseButtonDown(
		InGeometry,
		InMouseEvent
	);
}

FReply UAshenPlayerHUDWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent
)
{
	if (bStartupMenuOpen)
	{
		const FKey PressedKey =
			InKeyEvent.GetKey();

		if (PressedKey == EKeys::Enter ||
			PressedKey == EKeys::SpaceBar)
		{
			HandleBeginHuntClicked();
			return FReply::Handled();
		}

		if (PressedKey == EKeys::C)
		{
			HandleControlsClicked();
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(
		InGeometry,
		InKeyEvent
	);
}

bool UAshenPlayerHUDWidget::
	IsScreenPositionInsideWidget(
		const UWidget* Widget,
		const FVector2D& ScreenPosition
	) const
{
	if (!Widget ||
		Widget->GetVisibility() ==
			ESlateVisibility::Collapsed ||
		Widget->GetVisibility() ==
			ESlateVisibility::Hidden)
	{
		return false;
	}

	const FGeometry WidgetGeometry =
		Widget->GetCachedGeometry();

	const FVector2D LocalPosition =
		WidgetGeometry.AbsoluteToLocal(
			ScreenPosition
		);

	const FVector2D LocalSize =
		WidgetGeometry.GetLocalSize();

	return LocalPosition.X >= 0.0f &&
		LocalPosition.Y >= 0.0f &&
		LocalPosition.X <= LocalSize.X &&
		LocalPosition.Y <= LocalSize.Y;
}

void UAshenPlayerHUDWidget::
	ResolveAttributeComponent()
{
	if (ObservedAttributes)
	{
		return;
	}

	const AAshenPlayerCharacter* Player =
		Cast<AAshenPlayerCharacter>(
			GetOwningPlayerPawn()
		);

	if (!Player)
	{
		return;
	}

	ObservedAttributes =
		Player->GetAttributeComponent();
}

void UAshenPlayerHUDWidget::BuildInterface()
{
	if (bInterfaceBuilt || !WidgetTree)
	{
		return;
	}

	UCanvasPanel* RootCanvas =
		Cast<UCanvasPanel>(
			WidgetTree->RootWidget
		);

	if (!RootCanvas)
	{
		RootCanvas =
			WidgetTree->ConstructWidget<UCanvasPanel>(
				UCanvasPanel::StaticClass(),
				TEXT("AshenHUD_Root")
			);

		if (!RootCanvas)
		{
			return;
		}

		WidgetTree->RootWidget = RootCanvas;
	}
	else
	{
		/*
		 * Remove the old prototype bars from WBP_PlayerHUD.
		 * The final HUD is rebuilt entirely in native C++.
		 */
		RootCanvas->ClearChildren();
	}

	RootCanvas->SetIsEnabled(true);
	RootCanvas->SetVisibility(ESlateVisibility::Visible);

	/*
	 * Bottom-left vampire status panel.
	 */
	UBorder* StatusPanel = CreateBorder(
		TEXT("AshenHUD_StatusPanel"),
		AshenHUD::PanelColor,
		FMargin(
			18.0f,
			14.0f,
			18.0f,
			16.0f
		)
	);

	UVerticalBox* StatusColumn =
		WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("AshenHUD_StatusColumn")
		);

	StatusPanel->SetContent(StatusColumn);

	AddVerticalChild(
		StatusColumn,
		CreateText(
			TEXT("AshenHUD_Title"),
			TEXT("ASHEN KEEP  //  BLOOD HUNT"),
			13,
			AshenHUD::GoldColor,
			160
		),
		FMargin(
			0.0f,
			0.0f,
			0.0f,
			3.0f
		)
	);

	AddVerticalChild(
		StatusColumn,
		CreateText(
			TEXT("AshenHUD_Subtitle"),
			TEXT("VAMPIRE VITALS"),
			10,
			AshenHUD::MutedTextColor,
			220
		),
		FMargin(
			0.0f,
			0.0f,
			0.0f,
			11.0f
		)
	);

	AddVerticalChild(
		StatusColumn,
		CreateStatRow(
			TEXT("Health"),
			TEXT("HEALTH"),
			RuntimeHealthBar,
			AshenHUD::HealthColor
		),
		FMargin(
			0.0f,
			0.0f,
			0.0f,
			7.0f
		)
	);

	AddVerticalChild(
		StatusColumn,
		CreateStatRow(
			TEXT("Stamina"),
			TEXT("STAMINA"),
			RuntimeStaminaBar,
			AshenHUD::StaminaColor
		),
		FMargin(
			0.0f,
			0.0f,
			0.0f,
			7.0f
		)
	);

	AddVerticalChild(
		StatusColumn,
		CreateStatRow(
			TEXT("Blood"),
			TEXT("BLOOD ESSENCE"),
			RuntimeBloodBar,
			AshenHUD::BloodColor
		)
	);

	UCanvasPanelSlot* StatusCanvasSlot =
		RootCanvas->AddChildToCanvas(
			StatusPanel
		);

	StatusCanvasSlot->SetAnchors(
		FAnchors(
			0.0f,
			1.0f,
			0.0f,
			1.0f
		)
	);

	StatusCanvasSlot->SetAlignment(
		FVector2D(
			0.0f,
			1.0f
		)
	);

	StatusCanvasSlot->SetPosition(
		FVector2D(
			32.0f,
			-32.0f
		)
	);

	StatusCanvasSlot->SetSize(
		FVector2D(
			410.0f,
			140.0f
		)
	);

	StatusCanvasSlot->SetZOrder(20);

	/*
	 * Top-center objective panel.
	 */
	UBorder* ObjectivePanel = CreateBorder(
		TEXT("AshenHUD_ObjectivePanel"),
		AshenHUD::SoftPanelColor,
		FMargin(
			18.0f,
			10.0f,
			18.0f,
			11.0f
		)
	);

	UVerticalBox* ObjectiveColumn =
		WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("AshenHUD_ObjectiveColumn")
		);

	ObjectivePanel->SetContent(
		ObjectiveColumn
	);

	UTextBlock* ObjectiveTitle = CreateText(
		TEXT("AshenHUD_ObjectiveTitle"),
		TEXT("PURGE RITUAL"),
		11,
		AshenHUD::GoldColor,
		220
	);

	ObjectiveTitle->SetJustification(
		ETextJustify::Center
	);

	AddVerticalChild(
		ObjectiveColumn,
		ObjectiveTitle
	);

	UTextBlock* ObjectiveText = CreateText(
		TEXT("AshenHUD_ObjectiveText"),
		TEXT(
			"Defeat the Hunter Captain  |  Reach the altar"
		),
		15,
		AshenHUD::MainTextColor,
		35
	);

	ObjectiveText->SetJustification(
		ETextJustify::Center
	);

	AddVerticalChild(
		ObjectiveColumn,
		ObjectiveText,
		FMargin(
			0.0f,
			4.0f,
			0.0f,
			0.0f
		)
	);

	UCanvasPanelSlot* ObjectiveCanvasSlot =
		RootCanvas->AddChildToCanvas(
			ObjectivePanel
		);

	ObjectiveCanvasSlot->SetAnchors(
		FAnchors(
			0.5f,
			0.0f,
			0.5f,
			0.0f
		)
	);

	ObjectiveCanvasSlot->SetAlignment(
		FVector2D(
			0.5f,
			0.0f
		)
	);

	ObjectiveCanvasSlot->SetPosition(
		FVector2D(
			0.0f,
			22.0f
		)
	);

	ObjectiveCanvasSlot->SetSize(
		FVector2D(
			450.0f,
			56.0f
		)
	);

	ObjectiveCanvasSlot->SetZOrder(18);

	/*
	 * Bottom-right compact controls panel.
	 */
	UBorder* ControlsPanel = CreateBorder(
		TEXT("AshenHUD_ControlsPanel"),
		AshenHUD::SoftPanelColor,
		FMargin(
			14.0f,
			10.0f,
			14.0f,
			11.0f
		)
	);

	UVerticalBox* ControlsColumn =
		WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("AshenHUD_ControlsColumn")
		);

	ControlsPanel->SetContent(
		ControlsColumn
	);

	AddVerticalChild(
		ControlsColumn,
		CreateText(
			TEXT("AshenHUD_ControlsTitle"),
			TEXT("COMBAT"),
			10,
			AshenHUD::GoldColor,
			220
		),
		FMargin(
			0.0f,
			0.0f,
			0.0f,
			4.0f
		)
	);

	AddVerticalChild(
		ControlsColumn,
		CreateText(
			TEXT("AshenHUD_ControlsLineOne"),
			TEXT(
				"LMB  ATTACK     Q  BLOOD BURST"
			),
			11,
			AshenHUD::MainTextColor,
			45
		),
		FMargin(
			0.0f,
			0.0f,
			0.0f,
			3.0f
		)
	);

	AddVerticalChild(
		ControlsColumn,
		CreateText(
			TEXT("AshenHUD_ControlsLineTwo"),
			TEXT(
				"TAB  LOCK ON     CTRL  MIST STEP"
			),
			11,
			AshenHUD::MutedTextColor,
			45
		),
		FMargin(
			0.0f,
			0.0f,
			0.0f,
			3.0f
		)
	);

	AddVerticalChild(
		ControlsColumn,
		CreateText(
			TEXT("AshenHUD_ControlsLineThree"),
			TEXT(
				"SHIFT  SPRINT     SPACE  JUMP"
			),
			11,
			AshenHUD::MutedTextColor,
			45
		)
	);

	UCanvasPanelSlot* ControlsCanvasSlot =
		RootCanvas->AddChildToCanvas(
			ControlsPanel
		);

	ControlsCanvasSlot->SetAnchors(
		FAnchors(
			1.0f,
			1.0f,
			1.0f,
			1.0f
		)
	);

	ControlsCanvasSlot->SetAlignment(
		FVector2D(
			1.0f,
			1.0f
		)
	);

	ControlsCanvasSlot->SetPosition(
		FVector2D(
			-32.0f,
			-32.0f
		)
	);

	ControlsCanvasSlot->SetSize(
		FVector2D(
			340.0f,
			104.0f
		)
	);

	ControlsCanvasSlot->SetZOrder(19);

	BuildStartupMenu(RootCanvas);

	bInterfaceBuilt = true;
}

void UAshenPlayerHUDWidget::BuildStartupMenu(
	UCanvasPanel* RootCanvas
)
{
	if (!RootCanvas || StartupMenuOverlay)
	{
		return;
	}

	StartupMenuOverlay = CreateBorder(
		TEXT("AshenMenu_Overlay"),
		AshenHUD::MenuBackgroundColor,
		FMargin(0.0f)
	);

	UCanvasPanelSlot* OverlayCanvasSlot =
		RootCanvas->AddChildToCanvas(
			StartupMenuOverlay
		);

	OverlayCanvasSlot->SetAnchors(
		FAnchors(
			0.0f,
			0.0f,
			1.0f,
			1.0f
		)
	);

	OverlayCanvasSlot->SetOffsets(
		FMargin(
			0.0f,
			0.0f,
			0.0f,
			0.0f
		)
	);

	OverlayCanvasSlot->SetZOrder(1000);

	UCanvasPanel* MenuCanvas =
		WidgetTree->ConstructWidget<UCanvasPanel>(
			UCanvasPanel::StaticClass(),
			TEXT("AshenMenu_Canvas")
		);

	StartupMenuOverlay->SetContent(
		MenuCanvas
	);

	UBorder* MenuPanel = CreateBorder(
		TEXT("AshenMenu_Panel"),
		AshenHUD::MenuPanelColor,
		FMargin(
			42.0f,
			34.0f,
			42.0f,
			34.0f
		)
	);

	UCanvasPanelSlot* MenuPanelCanvasSlot =
		MenuCanvas->AddChildToCanvas(
			MenuPanel
		);

	MenuPanelCanvasSlot->SetAnchors(
		FAnchors(
			0.5f,
			0.5f,
			0.5f,
			0.5f
		)
	);

	MenuPanelCanvasSlot->SetAlignment(
		FVector2D(
			0.5f,
			0.5f
		)
	);

	MenuPanelCanvasSlot->SetPosition(
		FVector2D::ZeroVector
	);

	MenuPanelCanvasSlot->SetSize(
		FVector2D(
			520.0f,
			610.0f
		)
	);

	UVerticalBox* MenuColumn =
		WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("AshenMenu_Column")
		);

	MenuPanel->SetContent(
		MenuColumn
	);

	UTextBlock* TitleText = CreateText(
		TEXT("AshenMenu_Title"),
		TEXT("ASHEN KEEP"),
		42,
		AshenHUD::MainTextColor,
		230
	);

	TitleText->SetJustification(
		ETextJustify::Center
	);

	AddVerticalChild(
		MenuColumn,
		TitleText
	);

	UTextBlock* SubtitleText = CreateText(
		TEXT("AshenMenu_Subtitle"),
		TEXT("BLOOD HUNT"),
		17,
		AshenHUD::GoldColor,
		380
	);

	SubtitleText->SetJustification(
		ETextJustify::Center
	);

	AddVerticalChild(
		MenuColumn,
		SubtitleText,
		FMargin(
			0.0f,
			7.0f,
			0.0f,
			5.0f
		)
	);

	UTextBlock* TaglineText = CreateText(
		TEXT("AshenMenu_Tagline"),
		TEXT(
			"Take back your castle before dawn."
		),
		13,
		AshenHUD::MutedTextColor,
		35
	);

	TaglineText->SetJustification(
		ETextJustify::Center
	);

	AddVerticalChild(
		MenuColumn,
		TaglineText,
		FMargin(
			0.0f,
			0.0f,
			0.0f,
			32.0f
		)
	);

	StartupBeginButton = CreateMenuButton(
		TEXT("AshenMenu_BeginButton"),
		TEXT("BEGIN THE HUNT")
	);

	AddVerticalChild(
		MenuColumn,
		StartupBeginButton,
		FMargin(
			0.0f,
			0.0f,
			0.0f,
			12.0f
		)
	);

	StartupControlsButton = CreateMenuButton(
		TEXT("AshenMenu_ControlsButton"),
		TEXT("CONTROLS")
	);

	AddVerticalChild(
		MenuColumn,
		StartupControlsButton,
		FMargin(
			0.0f,
			0.0f,
			0.0f,
			12.0f
		)
	);

	StartupQuitButton = CreateMenuButton(
		TEXT("AshenMenu_QuitButton"),
		TEXT("QUIT")
	);

	AddVerticalChild(
		MenuColumn,
		StartupQuitButton,
		FMargin(
			0.0f,
			0.0f,
			0.0f,
			18.0f
		)
	);

	StartupControlsPanel = CreateBorder(
		TEXT("AshenMenu_ControlsPanel"),
		FLinearColor(
			0.040f,
			0.025f,
			0.035f,
			0.98f
		),
		FMargin(
			18.0f,
			14.0f,
			18.0f,
			14.0f
		)
	);

	UVerticalBox* MenuControlsColumn =
		WidgetTree->ConstructWidget<UVerticalBox>(
			UVerticalBox::StaticClass(),
			TEXT("AshenMenu_ControlsColumn")
		);

	StartupControlsPanel->SetContent(
		MenuControlsColumn
	);

	UTextBlock* MenuControlsTitle = CreateText(
		TEXT("AshenMenu_ControlsTitle"),
		TEXT("VAMPIRE COMMANDS"),
		11,
		AshenHUD::GoldColor,
		180
	);

	MenuControlsTitle->SetJustification(
		ETextJustify::Center
	);

	AddVerticalChild(
		MenuControlsColumn,
		MenuControlsTitle,
		FMargin(
			0.0f,
			0.0f,
			0.0f,
			7.0f
		)
	);

	UTextBlock* MenuControlsText = CreateText(
		TEXT("AshenMenu_ControlsText"),
		TEXT(
			"WASD  MOVE     LMB  ATTACK\n"
			"Q  BLOOD BURST     CTRL  MIST STEP\n"
			"TAB  LOCK ON     SHIFT  SPRINT     SPACE  JUMP"
		),
		12,
		AshenHUD::MainTextColor,
		40
	);

	MenuControlsText->SetJustification(
		ETextJustify::Center
	);

	AddVerticalChild(
		MenuControlsColumn,
		MenuControlsText
	);

	StartupControlsPanel->SetVisibility(
		ESlateVisibility::Collapsed
	);

	AddVerticalChild(
		MenuColumn,
		StartupControlsPanel
	);

	UTextBlock* FooterText = CreateText(
		TEXT("AshenMenu_Footer"),
		TEXT(
			"C++ / Unreal Engine vertical slice"
		),
		10,
		AshenHUD::MutedTextColor,
		65
	);

	FooterText->SetJustification(
		ETextJustify::Center
	);

	AddVerticalChild(
		MenuColumn,
		FooterText,
		FMargin(
			0.0f,
			18.0f,
			0.0f,
			0.0f
		)
	);
}

void UAshenPlayerHUDWidget::ShowStartupMenu()
{
	if (!StartupMenuOverlay)
	{
		return;
	}

	SetIsEnabled(true);
	SetVisibility(ESlateVisibility::Visible);

	StartupMenuOverlay->SetIsEnabled(true);
	StartupMenuOverlay->SetVisibility(
		ESlateVisibility::Visible
	);

	if (StartupBeginButton)
	{
		StartupBeginButton->SetIsEnabled(true);
		StartupBeginButton->SetVisibility(
			ESlateVisibility::Visible
		);
	}

	bStartupMenuOpen = true;
	bMenuInputApplied = false;

	ApplyStartupMenuInput();
}

void UAshenPlayerHUDWidget::
	ApplyStartupMenuInput()
{
	APlayerController* PlayerController =
		GetOwningPlayer();

	if (!PlayerController ||
		!PlayerController->IsLocalController())
	{
		return;
	}

	/*
	 * Make mouse events explicit. The HUD Blueprint originally existed
	 * as a non-interactive overlay, so relying on inherited defaults can
	 * leave runtime buttons visible but unable to receive clicks.
	 */
	PlayerController->bEnableClickEvents = true;
	PlayerController->bEnableMouseOverEvents = true;

	PlayerController->SetShowMouseCursor(
		true
	);

	PlayerController->SetIgnoreMoveInput(
		true
	);

	PlayerController->SetIgnoreLookInput(
		true
	);

	FInputModeGameAndUI InputMode;

	InputMode.SetWidgetToFocus(
		TakeWidget()
	);

	InputMode.SetLockMouseToViewportBehavior(
		EMouseLockMode::DoNotLock
	);

	InputMode.SetHideCursorDuringCapture(
		false
	);

	PlayerController->SetInputMode(
		InputMode
	);

	if (StartupBeginButton)
	{
		StartupBeginButton->SetUserFocus(
			PlayerController
		);
	}

	/*
	 * Pause only after the input mode and cursor have been established.
	 * Slate/UMG continues to process button clicks while the world is paused.
	 */
	UGameplayStatics::SetGamePaused(
		this,
		true
	);

	bMenuInputApplied = true;
}

void UAshenPlayerHUDWidget::CloseStartupMenu()
{
	if (StartupMenuOverlay)
	{
		StartupMenuOverlay->SetVisibility(
			ESlateVisibility::Collapsed
		);
	}

	APlayerController* PlayerController =
		GetOwningPlayer();

	UGameplayStatics::SetGamePaused(
		this,
		false
	);

	if (PlayerController)
	{
		PlayerController->SetIgnoreMoveInput(
			false
		);

		PlayerController->SetIgnoreLookInput(
			false
		);

		PlayerController->bEnableClickEvents = false;
		PlayerController->bEnableMouseOverEvents = false;

		PlayerController->SetShowMouseCursor(
			false
		);

		FInputModeGameOnly InputMode;

		PlayerController->SetInputMode(
			InputMode
		);
	}

	bStartupMenuOpen = false;
	bMenuInputApplied = false;
}

UTextBlock* UAshenPlayerHUDWidget::CreateText(
	const FName WidgetName,
	const FString& InText,
	const int32 FontSize,
	const FLinearColor& InColor,
	const int32 LetterSpacing
)
{
	UTextBlock* TextWidget =
		WidgetTree->ConstructWidget<UTextBlock>(
			UTextBlock::StaticClass(),
			WidgetName
		);

	TextWidget->SetText(
		FText::FromString(InText)
	);

	TextWidget->SetColorAndOpacity(
		FSlateColor(InColor)
	);

	FSlateFontInfo FontInfo =
		TextWidget->GetFont();

	FontInfo.Size = FontSize;
	FontInfo.LetterSpacing =
		LetterSpacing;

	TextWidget->SetFont(
		FontInfo
	);

	TextWidget->SetShadowOffset(
		FVector2D(
			1.0f,
			1.0f
		)
	);

	TextWidget->SetShadowColorAndOpacity(
		FLinearColor(
			0.0f,
			0.0f,
			0.0f,
			0.65f
		)
	);

	return TextWidget;
}

UBorder* UAshenPlayerHUDWidget::CreateBorder(
	const FName WidgetName,
	const FLinearColor& InColor,
	const FMargin& InPadding
)
{
	UBorder* BorderWidget =
		WidgetTree->ConstructWidget<UBorder>(
			UBorder::StaticClass(),
			WidgetName
		);

	BorderWidget->SetBrushColor(
		InColor
	);

	BorderWidget->SetPadding(
		InPadding
	);

	BorderWidget->SetHorizontalAlignment(
		HAlign_Fill
	);

	BorderWidget->SetVerticalAlignment(
		VAlign_Fill
	);

	return BorderWidget;
}

UButton* UAshenPlayerHUDWidget::CreateMenuButton(
	const FName WidgetName,
	const FString& LabelText
)
{
	UButton* ButtonWidget =
		WidgetTree->ConstructWidget<UButton>(
			UButton::StaticClass(),
			WidgetName
		);

	ButtonWidget->SetIsEnabled(true);
	ButtonWidget->SetVisibility(
		ESlateVisibility::Visible
	);

	ButtonWidget->SetBackgroundColor(
		AshenHUD::MenuButtonColor
	);

	USizeBox* ButtonSize =
		WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			FName(
				*FString::Printf(
					TEXT("%s_Size"),
					*WidgetName.ToString()
				)
			)
		);

	ButtonSize->SetHeightOverride(
		52.0f
	);

	UTextBlock* ButtonLabel = CreateText(
		FName(
			*FString::Printf(
				TEXT("%s_Label"),
				*WidgetName.ToString()
			)
		),
		LabelText,
		15,
		AshenHUD::MainTextColor,
		150
	);

	ButtonLabel->SetJustification(
		ETextJustify::Center
	);

	ButtonLabel->SetVisibility(
		ESlateVisibility::HitTestInvisible
	);

	ButtonSize->SetVisibility(
		ESlateVisibility::HitTestInvisible
	);

	ButtonSize->SetContent(
		ButtonLabel
	);

	ButtonWidget->SetContent(
		ButtonSize
	);

	return ButtonWidget;
}

UWidget* UAshenPlayerHUDWidget::CreateStatRow(
	const FName RowName,
	const FString& LabelText,
	UProgressBar*& OutProgressBar,
	const FLinearColor& FillColor
)
{
	UHorizontalBox* RowBox =
		WidgetTree->ConstructWidget<UHorizontalBox>(
			UHorizontalBox::StaticClass(),
			FName(
				*FString::Printf(
					TEXT("AshenHUD_%sRow"),
					*RowName.ToString()
				)
			)
		);

	USizeBox* LabelBox =
		WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			FName(
				*FString::Printf(
					TEXT("AshenHUD_%sLabelBox"),
					*RowName.ToString()
				)
			)
		);

	LabelBox->SetWidthOverride(
		110.0f
	);

	LabelBox->SetHeightOverride(
		22.0f
	);

	LabelBox->SetContent(
		CreateText(
			FName(
				*FString::Printf(
					TEXT("AshenHUD_%sLabel"),
					*RowName.ToString()
				)
			),
			LabelText,
			12,
			AshenHUD::MainTextColor,
			90
		)
	);

	UHorizontalBoxSlot* LabelBoxSlot =
		RowBox->AddChildToHorizontalBox(
			LabelBox
		);

	LabelBoxSlot->SetPadding(
		FMargin(
			0.0f,
			0.0f,
			10.0f,
			0.0f
		)
	);

	LabelBoxSlot->SetVerticalAlignment(
		VAlign_Center
	);

	UBorder* BarOutline = CreateBorder(
		FName(
			*FString::Printf(
				TEXT("AshenHUD_%sBarOutline"),
				*RowName.ToString()
			)
		),
		AshenHUD::AccentColor,
		FMargin(1.0f)
	);

	UBorder* BarBackground = CreateBorder(
		FName(
			*FString::Printf(
				TEXT("AshenHUD_%sBarBackground"),
				*RowName.ToString()
			)
		),
		FLinearColor(
			0.035f,
			0.028f,
			0.035f,
			0.98f
		),
		FMargin(2.0f)
	);

	USizeBox* BarSize =
		WidgetTree->ConstructWidget<USizeBox>(
			USizeBox::StaticClass(),
			FName(
				*FString::Printf(
					TEXT("AshenHUD_%sBarSize"),
					*RowName.ToString()
				)
			)
		);

	BarSize->SetWidthOverride(
		230.0f
	);

	BarSize->SetHeightOverride(
		12.0f
	);

	OutProgressBar =
		WidgetTree->ConstructWidget<UProgressBar>(
			UProgressBar::StaticClass(),
			FName(
				*FString::Printf(
					TEXT("AshenHUD_%sProgress"),
					*RowName.ToString()
				)
			)
		);

	OutProgressBar->SetPercent(
		1.0f
	);

	OutProgressBar->SetFillColorAndOpacity(
		FillColor
	);

	OutProgressBar->SetBorderPadding(
		FVector2D::ZeroVector
	);

	BarSize->SetContent(
		OutProgressBar
	);

	BarBackground->SetContent(
		BarSize
	);

	BarOutline->SetContent(
		BarBackground
	);

	UHorizontalBoxSlot* BarBoxSlot =
		RowBox->AddChildToHorizontalBox(
			BarOutline
		);

	BarBoxSlot->SetVerticalAlignment(
		VAlign_Center
	);

	return RowBox;
}

void UAshenPlayerHUDWidget::AddVerticalChild(
	UVerticalBox* ParentBox,
	UWidget* ChildWidget,
	const FMargin& InPadding
)
{
	if (!ParentBox || !ChildWidget)
	{
		return;
	}

	UVerticalBoxSlot* VerticalBoxSlot =
		ParentBox->AddChildToVerticalBox(
			ChildWidget
		);

	VerticalBoxSlot->SetPadding(
		InPadding
	);

	VerticalBoxSlot->SetHorizontalAlignment(
		HAlign_Fill
	);
}

void UAshenPlayerHUDWidget::
	HandleBeginHuntClicked()
{
	CloseStartupMenu();
}

void UAshenPlayerHUDWidget::
	HandleControlsClicked()
{
	if (!StartupControlsPanel)
	{
		return;
	}

	const bool bControlsVisible =
		StartupControlsPanel->GetVisibility() ==
			ESlateVisibility::Visible;

	StartupControlsPanel->SetVisibility(
		bControlsVisible
			? ESlateVisibility::Collapsed
			: ESlateVisibility::Visible
	);
}

void UAshenPlayerHUDWidget::
	HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(
		this,
		GetOwningPlayer(),
		EQuitPreference::Quit,
		false
	);
}

void UAshenPlayerHUDWidget::RefreshBars()
{
	if (!ObservedAttributes)
	{
		return;
	}

	if (RuntimeHealthBar)
	{
		RuntimeHealthBar->SetPercent(
			AshenHUD::SafePercent(
				ObservedAttributes->GetHealth(),
				ObservedAttributes->GetMaxHealth()
			)
		);
	}

	if (RuntimeStaminaBar)
	{
		RuntimeStaminaBar->SetPercent(
			AshenHUD::SafePercent(
				ObservedAttributes->GetStamina(),
				ObservedAttributes->GetMaxStamina()
			)
		);
	}

	if (RuntimeBloodBar)
	{
		RuntimeBloodBar->SetPercent(
			AshenHUD::SafePercent(
				ObservedAttributes->GetMana(),
				ObservedAttributes->GetMaxMana()
			)
		);
	}
}
