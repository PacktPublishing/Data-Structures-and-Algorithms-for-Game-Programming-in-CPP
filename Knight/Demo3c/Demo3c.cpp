#include "Knight.h"
#include "Demo3c.h"
#include "Random.h"
#include "Shuffler.h"

#define IMAGE_FILENAME_BUFFER_SIZE 64
#define DECK_CARD_COUNT 52
#define PICK_CARDS_COUNT 13
int CardIDs[DECK_CARD_COUNT];
Texture2D CardImages[DECK_CARD_COUNT];
int PickedCardIDs[PICK_CARDS_COUNT];

int main(int argc, char* argv[])
{
	Demo3c* demo3c = new Demo3c();
	demo3c->Start();
	demo3c->GameLoop();
	delete demo3c;
	return 0;
}

void Demo3c::Start()
{
	char imageFile[IMAGE_FILENAME_BUFFER_SIZE];

	__super::Start();

	Config.ShowFPS = true;

	//Initialize card IDs and load card images
	for (int i = 0; i < DECK_CARD_COUNT; ++i)
	{
		//Initialize the card ID
		CardIDs[i] = i;

		if (i < PICK_CARDS_COUNT)
		{
			PickedCardIDs[i] = i;
		}

		// Load directly: on Android these files are APK assets, so raylib's
		// filesystem-only FileExists() cannot see them even though LoadTexture()
		// can read them through AAssetManager.
		sprintf_s(imageFile, IMAGE_FILENAME_BUFFER_SIZE, (Config.ResourcesBasePath + "textures/PokerDeckCards/%d.png").c_str(), i);
		CardImages[i] = LoadTextureForPlatform(imageFile);
		if (CardImages[i].id == 0)
			TraceLog(LOG_ERROR, "Demo3c: failed to load card texture: %s", imageFile);
	}
}

void Demo3c::EndGame()
{
	for (int i = 0; i < DECK_CARD_COUNT; ++i)
	{
		if (CardImages[i].id != 0)
			UnloadTexture(CardImages[i]);
	}
	__super::EndGame();
}


void Demo3c::Update(float ElapsedSeconds)
{
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		Shuffle();
	}

	__super::Update(ElapsedSeconds);
}

void Demo3c::DrawGUI()
{
	DrawText("Left Click: Shuffle the deck of cards.", 100, 50, 30, GREEN);

	int x = 15;
	for (int i = 0; i < PICK_CARDS_COUNT; ++i)
	{
		DrawTexture(CardImages[PickedCardIDs[i]], x, 180, WHITE);
		x += 120;
	}
}

void Demo3c::Shuffle()
{
	Shuffler::FisherYateShuffle(CardIDs, DECK_CARD_COUNT, PickedCardIDs, PICK_CARDS_COUNT);
}
