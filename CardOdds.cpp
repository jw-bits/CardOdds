// CardOdds.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <chrono>
#include <random>
#include <algorithm>
#include <array>
#include <map>

enum ESuit
{
	eHeart = 1,
	eSpade,
	eClub,
	eDiamond
};

enum EValue
{
	eAce = 1,
	eTwo,
	eThree,
	eFour,
	eFive,
	eSix,
	eSeven,
	eEight,
	eNine,
	eTen,
	eJack,
	eQueen,
	eKing
};

// The first momement a player would know they had a guarented winning or push hand
enum eWinOrPushObserved
{
	eNA = 0,
	ePocketCards,
	eFirstRiverCard,
	eSecondRiverCard,
	eThirdRiverCard
};

enum eResultType
{
	eLoss = -1,
	ePush = 0,
	ePairOfJacksOrHigher,
	eTwoPair,
	eThreeOfAKind,
	eStraight,
	eFlush,
	eFullHouse,
	eFourOfAKind,
	eStraightFlush,
	eRoyalFlush,
};

class CCard
{
public:
	CCard() { }

	CCard(ESuit s, EValue v) {
		mSuit = s; mValue = v;
	}

	ESuit mSuit;
	EValue mValue;
};

struct SDealer
{
	int mHand[3];
};

struct SPlayer
{
	int mHand[2];
};

class CHandEval
{
public:

	CHandEval() {
		mObserved = eNA;
		mResult = eLoss;
	}

	bool isPushOrWin() {
		return mResult != eLoss;
	}

	void set(eWinOrPushObserved o, eResultType rt) {
		mObserved = o;
		mResult = rt;
	}

	eWinOrPushObserved getObserved() const { return mObserved; }
	eResultType getResult() const { return mResult; }

private:
	eWinOrPushObserved mObserved;
	eResultType mResult;
};

class CStats
{
public:
	CStats() { }
	CStats(const CCard &c0, const CCard &c1, eWinOrPushObserved o, int dealtGame)
	{
		mCard0 = c0;
		mCard1 = c1;
		mObserved = o;
		mDealtGame = dealtGame;
	}
	
	bool operator <(const CStats &right) const {
		int v1 = (mCard0.mSuit * 100 + mCard0.mValue) + (mCard1.mSuit * 1000 + mCard1.mValue) + mObserved * 10000;
		int v2 = (right.mCard0.mSuit * 100 + right.mCard0.mValue) + (right.mCard1.mSuit * 1000 + right.mCard1.mValue) + right.mObserved * 10000;
		return (v1 < v2);
	}

	const CCard& getCard0() const { return mCard0; }
	const CCard& getCard1() const { return mCard1; }
	const eWinOrPushObserved getObserved() const { return mObserved; }

private:
	CCard mCard0;
	CCard mCard1;
	eWinOrPushObserved mObserved;
	int mDealtGame;
};

const int kCardsInDeck = 52;
const int kMaxPlayers = 5;
const int kPokerHand = 5;

CCard gDeck[kCardsInDeck];
SDealer gDealer;
SPlayer gPlayers[kMaxPlayers];
int gShuffledDeck[kCardsInDeck];
std::map<CStats, int> gStats;

void SeedRandomGenerator();
void InitializeCards();
void PrintCard(const CCard &card);
void PrintResult(eResultType rt);
void PrintObserved(eWinOrPushObserved o);
void ShuffleCards(int shuffleCount);
void Deal(int playerCount);

// Hand evaluation
CHandEval Evaluate(const SPlayer &p);
void TopHandAnalysis();

int main()
{
	const int kActivePlayers = 2;
	const int kTotalGames = 1000000;
	const int kMaxResultTypes = eRoyalFlush + 1;

	SeedRandomGenerator();
	InitializeCards();

	int totalNoBrainers = 0;
	int totalFirstRiverCard = 0;
	int totalSecondRiverCard = 0;
	int totalThirdRiverCard = 0;
	
	int perHandTotals[kMaxResultTypes];
	int perPlayerWins[kActivePlayers];

	memset(perHandTotals, 0, sizeof(int) * kMaxResultTypes);
	memset(perPlayerWins, 0, sizeof(int) * kActivePlayers);

	int totalWins = 0;

	for (int game = 0; game < kTotalGames; ++game)
	{
		ShuffleCards(3);
		Deal(kActivePlayers);

		for (int i = 0; i < kActivePlayers; ++i)
		{
			CHandEval e = Evaluate(gPlayers[i]);

			if (e.isPushOrWin())
			{
				//// Print Dealer
				//printf("Dealers Cards:\n");
				//
				//for (int i = 0; i < 3; ++i)
				//	PrintCard(gDeck[gDealer.mHand[i]]);

				//// Print players
				//printf("\nPlayers Cards:\n");

				//for (int i = 0; i < kPlayers; ++i)
				//{
				//	PrintCard(gDeck[gPlayers[i].mHand[0]]);
				//	PrintCard(gDeck[gPlayers[i].mHand[1]]);
				//	printf("\n");
				//}

				if (e.getObserved() == ePocketCards)
					totalNoBrainers++;
				else if (e.getObserved() == eFirstRiverCard)
					totalFirstRiverCard++;
				else if (e.getObserved() == eSecondRiverCard)
					totalSecondRiverCard++;
				else if (e.getObserved() == eThirdRiverCard)
					totalThirdRiverCard++;

				perHandTotals[e.getResult()] += 1;

				if (e.getResult() != ePush)
				{
					perPlayerWins[i] += 1;
					totalWins++;
				}

				// Save stats
				CStats stat(gDeck[gPlayers[i].mHand[0]], gDeck[gPlayers[i].mHand[1]], e.getObserved(), game);

				std::map<CStats, int>::iterator iter = gStats.find(stat);

				if (iter == gStats.end())
					gStats.insert(std::pair<CStats, int>(stat, 1));
				else
					iter->second = iter->second + 1;
			}
		}
	}

	printf("Total no brainers: %d\n", totalNoBrainers);
	printf("Total first river card: %d\n", totalFirstRiverCard);
	printf("Total second river card: %d\n", totalSecondRiverCard);
	printf("Total third river card: %d\n", totalThirdRiverCard);
	printf("================================\n");
	printf("Total pair of Jacks or better: %d (%.2f%%)\n", perHandTotals[ePairOfJacksOrHigher], (perHandTotals[ePairOfJacksOrHigher] / (float)kTotalGames * 100.0f));
	printf("Total two pairs: %d (%.2f%%)\n", perHandTotals[eTwoPair], perHandTotals[eTwoPair] / (float)kTotalGames * 100.0f);
	printf("Total trips: %d (%.2f%%)\n", perHandTotals[eThreeOfAKind], perHandTotals[eThreeOfAKind] / (float)kTotalGames * 100.0f);
	printf("Total straights: %d (%.2f%%)\n", perHandTotals[eStraight], perHandTotals[eStraight] / (float)kTotalGames * 100.0f);
	printf("Total flushes: %d (%.2f%%)\n", perHandTotals[eFlush], perHandTotals[eFlush] / (float)kTotalGames * 100.0f);
	printf("Total full houses: %d (%.2f%%)\n", perHandTotals[eFullHouse], perHandTotals[eFullHouse] / (float)kTotalGames * 100.0f);
	printf("Total quads: %d (%.2f%%)\n", perHandTotals[eFourOfAKind], perHandTotals[eFourOfAKind] / (float)kTotalGames * 100.0f);
	printf("Total straight flushes: %d (%.2f%%)\n", perHandTotals[eStraightFlush], perHandTotals[eStraightFlush] / (float)kTotalGames * 100.0f);
	printf("Total royal flushes: %d (%.2f%%)\n", perHandTotals[eRoyalFlush], perHandTotals[eRoyalFlush] / (float)kTotalGames * 100.0f);
	printf("================================\n");
	printf("Total pushes or winning hands: %d\n", totalWins + perHandTotals[ePush]);
	printf("Total winning hands: %d\n", totalWins);
	
	for (int i = 0; i < kActivePlayers; ++i)
		printf("Total wins for Player %d is: %d\n", i + 1, perPlayerWins[i]);

	TopHandAnalysis();
}

void SeedRandomGenerator()
{
	unsigned int s = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
	std::srand(s);
}

void InitializeCards()
{
	int index = 0;

	for (int i = eHeart; i <= eDiamond; ++i)
	{
		for (int j = eAce; j <= eKing; ++j, ++index)
		{
			gDeck[index].mSuit = (ESuit)i;
			gDeck[index].mValue = (EValue)j;
		}
	}

	// Start the shuffled cards in a random order
	int used[kCardsInDeck];
	memset(used, 0, sizeof(int) * kCardsInDeck);

	for (int i = 0; i < kCardsInDeck; )
	{
		int c = rand() % kCardsInDeck;

		if (used[c] == 1)
			continue;
		else
		{
			gShuffledDeck[i] = c;
			used[c] = 1;
			++i;
		}
	}
}

void PrintCard(const CCard &card)
{
	switch (card.mValue)
	{
		case eAce: printf("Ace of "); break;
		case eTwo: printf("Two of "); break;
		case eThree: printf("Three of "); break;
		case eFour: printf("Four of "); break;
		case eFive: printf("Five of "); break;
		case eSix: printf("Six of "); break;
		case eSeven: printf("Seven of "); break;
		case eEight: printf("Eight of "); break;
		case eNine: printf("Nine of "); break;
		case eTen: printf("Ten of "); break;
		case eJack: printf("Jack of "); break;
		case eQueen: printf("Queen of "); break;
		case eKing: printf("King of "); break;
	}

	switch (card.mSuit)
	{
		case eHeart: printf("Hearts\n"); break;
		case eClub: printf("Clubs\n"); break;
		case eSpade: printf("Spades\n"); break;
		case eDiamond: printf("Diamonds\n"); break;
	}
}

void PrintResult(eResultType rt)
{
	switch (rt)
	{
		case eLoss: printf("Loss"); break;
		case ePush: printf("Push"); break;
		case ePairOfJacksOrHigher: printf("Pair of Jacks or better"); break;
		case eTwoPair: printf("Two pairs"); break;
		case eThreeOfAKind: printf("Three of a kind"); break;
		case eStraight: printf("Straight"); break;
		case eFlush: printf("Flush"); break;
		case eFullHouse: printf("Full House"); break;
		case eStraightFlush: printf("Straight Flush"); break;
		case eRoyalFlush: printf("Royal Flush"); break;
	}
}

void PrintObserved(eWinOrPushObserved o)
{
	switch (o)
	{
		case ePocketCards: printf("Pocket Cards"); break;
		case eFirstRiverCard: printf("First River Card"); break;
		case eSecondRiverCard: printf("Second River Card"); break;
		case eThirdRiverCard: printf("Third River Card"); break;
	}
}

void ShuffleCards(int shuffleCount)
{
	if (shuffleCount <= 0)
		return;

	for (int s = 0; s < shuffleCount; ++s)
	{
		// Shuffle
		for (int i = 0; i < kCardsInDeck; ++i)
		{
			int idx0 = rand() % kCardsInDeck;
			int idx1 = rand() % kCardsInDeck;

			std::swap(gShuffledDeck[idx0], gShuffledDeck[idx1]);
		}
	}	
}

void Deal(int playerCount)
{
	if (playerCount <= 0)
		return;
	else if (playerCount > kMaxPlayers)
		return;
	
	int cIdx = 0;

	// Give dealer 3 cards
	for (int i = 0; i < 3; ++i)
		gDealer.mHand[i] = gShuffledDeck[cIdx++];

	// Give each player 2 cards
	for (int i = 0; i < playerCount; ++i)
	{
		for (int j = 0; j < 2; ++j)
			gPlayers[i].mHand[j] = gShuffledDeck[cIdx++];
	}		
}

CHandEval Evaluate(const SPlayer &player)
{
	CHandEval e;
	
	const CCard &p0 = gDeck[player.mHand[0]];
	const CCard &p1 = gDeck[player.mHand[1]];
	const CCard &d0 = gDeck[gDealer.mHand[0]];
	const CCard &d1 = gDeck[gDealer.mHand[1]];
	const CCard &d2 = gDeck[gDealer.mHand[2]];

	std::array<int, kPokerHand> hand = { p0.mValue, p1.mValue, d0.mValue, d1.mValue, d2.mValue };
	std::sort(hand.begin(), hand.end(), std::less<int>());
	
	// Four of a Kind
	if (((hand[0] == hand[1]) && (hand[1] == hand[2]) && (hand[2] == hand[3])) || ((hand[1] == hand[2]) && (hand[2] == hand[3]) && (hand[3] == hand[4])))
	{
		if ((p0.mValue == p1.mValue) && (p0.mValue >= eSix))
			e.set(ePocketCards, eFourOfAKind);
		else if (d0.mValue >= eSix)
			e.set(eFirstRiverCard, eFourOfAKind);
		else
			e.set(eSecondRiverCard, eFourOfAKind);
	}
	// Full House
	else if (((hand[0] == hand[1]) && (hand[1] == hand[2]) && (hand[3] == hand[4])) || ((hand[0] == hand[1]) && (hand[2] == hand[3]) && (hand[3] == hand[4])))
	{
		if (p0.mValue == p1.mValue)
		{
			if (p0.mValue >= eSix)
				e.set(ePocketCards, eFullHouse);
			else if (d0.mValue == p0.mValue)
				e.set(eFirstRiverCard, eFullHouse);
			else
				e.set(eSecondRiverCard, eFullHouse);
		}
		else if (d0.mValue >= eSix) // Must pair with one of player's cards
			e.set(eFirstRiverCard, eFullHouse);
		else
			e.set(eSecondRiverCard, eFullHouse);
	}
	// Trips
	else if (((hand[0] == hand[1]) && (hand[1] == hand[2])) || ((hand[1] == hand[2]) && (hand[2] == hand[3])) || ((hand[2] == hand[3]) && (hand[3] == hand[4])))
	{
		if ((p0.mValue == p1.mValue) && (p0.mValue >= eSix))
			e.set(ePocketCards, eThreeOfAKind);
		else if (p0.mValue == p1.mValue)
		{
			if (d0.mValue == p0.mValue)
				e.set(eFirstRiverCard, eThreeOfAKind);
			else if (d1.mValue == p0.mValue)
				e.set(eSecondRiverCard, eThreeOfAKind);
			else
				e.set(eThirdRiverCard, eThreeOfAKind);
		}
		else if (((p0.mValue == d0.mValue) || (p1.mValue == d0.mValue)) && (d0.mValue >= eSix))
			e.set(eFirstRiverCard, eThreeOfAKind);
		else if (((p0.mValue == d1.mValue) || (p1.mValue == d1.mValue) || (d0.mValue == d1.mValue)) && (d1.mValue >= eSix))
			e.set(eSecondRiverCard, eThreeOfAKind);
		else
			e.set(eThirdRiverCard, eThreeOfAKind);
	}
	// Two Pair
	else if (((hand[0] == hand[1]) && (hand[2] == hand[3])) || ((hand[0] == hand[1]) && (hand[3] == hand[4])) || ((hand[1] == hand[2]) && (hand[3] == hand[4])))
	{
		if ((p0.mValue == p1.mValue) && (p0.mValue >= eSix))
			e.set(ePocketCards, eTwoPair);
		else if (p0.mValue == p1.mValue)
		{
			if (d0.mValue == d1.mValue)
				e.set(eSecondRiverCard, eTwoPair);
			else
				e.set(eThirdRiverCard, eTwoPair);
		}
		else if (((p0.mValue == d0.mValue) || (p1.mValue == d0.mValue)) && (d0.mValue >= eSix))
			e.set(eFirstRiverCard, eTwoPair);
		else if (((p0.mValue == d1.mValue) || (p1.mValue == d1.mValue)) && (d1.mValue >= eSix))
			e.set(eSecondRiverCard, eTwoPair);
		else
			e.set(eThirdRiverCard, eTwoPair);
	}
	// Pair
	else if ((hand[0] == hand[1]) || (hand[1] == hand[2]) || (hand[2] == hand[3]) || (hand[3] == hand[4]))
	{
		if ((p0.mValue == p1.mValue) && (p0.mValue >= eSix))
			e.set(ePocketCards, (p0.mValue >= eJack) ? ePairOfJacksOrHigher : ePush);
		else if (((p0.mValue == d0.mValue) || (p1.mValue == d0.mValue)) && (d0.mValue >= eSix))
			e.set(eFirstRiverCard, (d0.mValue >= eJack) ? ePairOfJacksOrHigher : ePush);
		else if (((p0.mValue == d1.mValue) || (p1.mValue == d1.mValue) || (d0.mValue == d1.mValue)) && (d1.mValue >= eSix))
			e.set(eSecondRiverCard, (d1.mValue >= eJack) ? ePairOfJacksOrHigher : ePush);
		else if (((p0.mValue == d2.mValue) || (p1.mValue == d2.mValue) || (d0.mValue == d2.mValue) || (d1.mValue == d2.mValue)) && (d2.mValue >= eSix))
			e.set(eThirdRiverCard, (d2.mValue >= eJack) ? ePairOfJacksOrHigher : ePush);
	}
	
	// Flush
	bool allSameSuit = (p0.mSuit == p1.mSuit) && (p1.mSuit == d0.mSuit) && (d0.mSuit == d1.mSuit) && (d1.mSuit == d2.mSuit);

	if (allSameSuit)
		e.set(eThirdRiverCard, eFlush);

	// Straights
	// StraightFlushes
	// RoyalFlushes
	bool hasStraight = false;

	// Aces's need special check as Ace can be first or last card in straight
	if ((hand[0] == eAce) && (hand[1] == eTwo) && (hand[2] == eThree) && (hand[3] == eFour) && (hand[4] == eFive))
		hasStraight = true;
	else if ((hand[0] == eAce) && (hand[1] == eTen) && (hand[2] == eJack) && (hand[3] == eQueen) && (hand[4] == eKing))
		hasStraight = true;
	else if ((hand[0] + 1 == hand[1]) && (hand[1] + 1 == hand[2]) && (hand[2] + 1 == hand[3]) && (hand[3] + 1 == hand[4]))
		hasStraight = true;
	
	if (hasStraight)
	{
		if (allSameSuit)
		{
			if ((hand[0] == eAce) && (hand[1] == eTen) && (hand[2] == eJack) && (hand[3] == eQueen) && (hand[4] == eKing))
				e.set(eThirdRiverCard, eRoyalFlush);
			else
				e.set(eThirdRiverCard, eStraightFlush);
		}
		else
			e.set(eThirdRiverCard, eStraight);
	}

	return e;
}

void TopHandAnalysis()
{
	// Find the top hands for either Pocket or FirstRiver wins
	const int kTopCount = 10;
	int topHandCounts[kTopCount] = { 0 };
	CStats topStats[kTopCount];

	for (std::map<CStats, int>::iterator iter = gStats.begin(); iter != gStats.end(); iter++)
	{
		int count = iter->second;

		for (int i = 0; i < kTopCount; ++i)
		{
			// Top hans for only pocket/first river cards
			//if ((count > topHandCounts[i]) && (iter->first.getObserved() == ePocketCards || iter->first.getObserved() == eFirstRiverCard))

			// Top hands
			if (count > topHandCounts[i])
			{
				topHandCounts[i] = count;
				topStats[i] = iter->first;
				break;
			}
		}
	}

	printf("\n!=== Top Hands ===!\n\n");
	for (int i = 0; i < kTopCount; ++i)
	{
		printf("Won or Pushed %d times: \n", topHandCounts[i]);
		PrintCard(topStats[i].getCard0());
		PrintCard(topStats[i].getCard1());
		printf("Observed: "); PrintObserved(topStats[i].getObserved()); printf("\n");
		printf("\n");
	}
}

