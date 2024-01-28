//---------------------------------------------------------------------------------------------------------------------
// a3.c
//
// A card game for two players which is loosely inspired by Ohanami.
//
// Group: Laurenz Fedotoff
//
// Author: 12309681
//---------------------------------------------------------------------------------------------------------------------
//


#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

const int MAX_ROW = 3;
const int MIN_ROW = 1;
const int MAX_CARD_PER_PLAYER = 10;
const int ERROR = -1;
const int OUT_OF_MEMORY = 4;
const int BUFFER_SIZE = 255;


typedef enum _Points_
{
  BLUE = 3,
  GREEN = 4,
  WHITE = 7,
  RED = 10
} Points;

typedef struct _Card_
{
  int number_;
  char color_;
  struct _Card_ *next_;
} Card;

typedef struct _Player_
{
  int index;
  struct _Card_ *hand_cards_;
  struct _Card_ *chosen_cards_;
  struct _Card_ **row_;
  int player_points_;
} Player;

typedef struct _Game_
{
  int amount_of_players_;
  int amount_of_cards_;
  char *file_name_;
} Game;

int parseConfigFile(char *file_name, Card **total_cards, Game *game);

int checkMagicNumber(FILE *config_file, char *file_name);

int getNumberOfPlayer(FILE *config_file);

void freeCardList(Card *head);

int cardDistribution(Player *players, Card *total_cards, Game *game);

void insertCardSorted(Card **HEAD, Card *new_card);

int runningGame(Player *players, Game *game);

void printPlayerStatusInfo(Player *players);

int cardChoosingPhase(Player *players, Game *game);

int stringCompareCaseInsensitive(const char *string1, const char *string2);

void removeCardFromHand(struct _Card_ **HEAD, Card *hand_card);

void swapCardDeck(Player *players, Game *game);

int actionPhase(Player *players, Game *game);

int actionPhaseCommands(char *string, Player *player);

int handlePlaceCommand(Player *player);

int handleHelpCommand(void);

int handleQuitCommand(void);

int handleDiscardCommand(Player *player);

int cardChosingPhaseCommands(char *string);

int placeCardInRow(int row, int number, Player *player);

int checkOutOfBound(int row);

int discardCard(char *strtok, Player *player);

int checkIfCardsLeft(struct _Card_ *chosen_cards, struct _Card_ *hand_cards);

void printPoints(Player *players, Game *game);

FILE *openFile(const char *file_name);

void printPlayerPoints(int player_index, int points);

void updatePlayerPoints(Player *players, int player_index, int index_longest_row, int total_points, int *highest_score);

void writePlayerPointsToFile(FILE *fp, int player_index, int points);

void printResults(Player *players, Game *game, int highest_score, FILE *fp);

int calculatePoints(int *counter, Card *temp, int total_points);

void freeMemory(Game *game, Player *player, Card *card);

size_t userInput(char **user_input);

void copyCardData(Card *source, Card *destination);

void handleCardChoosingPrompt(int numbers_entered, int error, int player_index);

int handleUserInput(char **input_buffer, int *result, int *error, int player_index);

int handleCardSelection(Player *player, Card **tmp, int *numbers_entered, int *error);

int initializeGame(int argc, char *argv[]);

void handleInvalidInput(Game *game, Player *players, Card *totalCards);

Player *initializePlayers(Game *game, Card *total_cards);

//---------------------------------------------------------------------------------------------------------------------
///
/// Entry and exitpoint of my program.
///
/// @param argc number of program arguments passed
/// @param argv arguments passed represented as string-array
///
/// @return exit code 0(success) - 4
//
int main(int argc, char *argv[])
{
  return initializeGame(argc, argv);
}

//---------------------------------------------------------------------------------------------------------------------
///
/// Creates and initializes two structs and passes them on. Handles different return values from other functions and
/// frees allocated memory accordingly.
///
/// @param argc number of program arguments passed
/// @param argv arguments passed represented as string-array
///
/// @return exit code 0(success) - 4
//
int initializeGame(int argc, char *argv[])
{
  Game *game = malloc(sizeof(Game));
  if (game == NULL)
  {
    printf("Error: Out of memory\n");
    return OUT_OF_MEMORY;
  }

  if (argc != 2)
  {
    printf("Usage: ./a3 <config file>\n");
    handleInvalidInput(game, NULL, NULL);
    return 1;
  }
  Card *totalCards;
  int result = parseConfigFile(argv[1], &totalCards, game);
  if (result != 0)
  {
    handleInvalidInput(game, NULL, NULL);
    return result;
  }

  printf("Welcome to SyntaxSakura (%d players are playing)!\n", game->amount_of_players_);

  Player *players = initializePlayers(game, totalCards);
  if (players == NULL)
  {
    handleInvalidInput(game, players, totalCards);
    return OUT_OF_MEMORY;
  }

  result = cardDistribution(players, totalCards, game);
  if (result == OUT_OF_MEMORY)
  {
    handleInvalidInput(game, players, totalCards);
    return OUT_OF_MEMORY;
  }

  result = runningGame(players, game);
  if (result == OUT_OF_MEMORY)
  {
    handleInvalidInput(game, players, totalCards);
    return OUT_OF_MEMORY;
  }

  freeMemory(game, players, totalCards);
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This function gets called when a user inputs wrong values. And calls the freeMemory function.
/// This function saves a lot of lines in my programm.
///
/// @param game strcut game(holds all important values for the game)
/// @param players array of struct Player
/// @param totalCards linked list of all the cards parsed from config file.
///
/// @return void
//
void handleInvalidInput(Game *game, Player *players, Card *totalCards)
{
  freeMemory(game, players, totalCards);
}

//---------------------------------------------------------------------------------------------------------------------
///
/// Initializes and creates struct Player dinamically.
///
/// @param game struct Game(holds all important values for the game)
/// @param totalCards linked list of all the cards parsed from config file.
///
/// @return players array of struct Player
//
Player *initializePlayers(Game *game, Card *total_cards)
{
  Player *players = malloc(sizeof(Player) * game->amount_of_players_);
  if (players == NULL)
  {
    printf("Error: Out of memory\n");
    return NULL;
  }

  for (int player_index = 0; player_index < game->amount_of_players_; ++player_index)
  {
    players[player_index].index = player_index;
    players[player_index].hand_cards_ = NULL;
    players[player_index].chosen_cards_ = NULL;
    players[player_index].row_ = malloc(sizeof(Card) * MAX_ROW);
    if (players[player_index].row_ == NULL)
    {
      printf("Error: Out of memory\n");
      freeMemory(game, players, total_cards);
      return NULL;
    }
    for (int row_index = 0; row_index < MAX_ROW; ++row_index)
    {
      players[player_index].row_[row_index] = NULL;
    }
  }

  return players;
}

//---------------------------------------------------------------------------------------------------------------------
///
/// Gets called at the end or when a memory alloc happens to clear all allocated memory to avoid memory leaks.
///
/// @param game struct Game(holds all important values for the game)
/// @param players array of struct Player
/// @param totalCards linked list of all the cards parsed from config file.
///
/// @return void
//
void freeMemory(Game *game, Player *player, Card *card)
{
  if (player != NULL && card != NULL)
  {
    for (int player_index = 0; player_index < game->amount_of_players_; ++player_index)
    {
      while (player[player_index].chosen_cards_ != NULL)
      {
        Card *temp = player[player_index].chosen_cards_;
        player[player_index].chosen_cards_ = temp->next_;
        free(temp);
      }
      while (player[player_index].hand_cards_ != NULL)
      {
        Card *temp = player[player_index].hand_cards_;
        player[player_index].hand_cards_ = temp->next_;
        free(temp);
      }
      for (int row_index = 0; row_index < MAX_ROW; ++row_index)
      {
        while (player[player_index].row_[row_index] != NULL)
        {
          Card *temp = player[player_index].row_[row_index];
          player[player_index].row_[row_index] = temp->next_;
          free(temp);
        }
      }
      free(player[player_index].row_);
    }
    free(player);
    while (card != NULL)
    {
      Card *temp = card;
      card = temp->next_;
      free(temp);
    }
    free(game->file_name_);
  }
  free(game);
}

//---------------------------------------------------------------------------------------------------------------------
///
/// Opens and reads the config file and parses all cards, amount of players and magic number.
///
/// @param file_name string of the entered file name
/// @param totalCards linked list of all the cards parsed from config file
/// @param game struct Game(holds all important values for the game)
///
/// @return int read successfully(0) or not(-1, 2, 4)
//
int parseConfigFile(char *file_name, Card **total_cards, Game *game)
{
  int confirmation;
  int people;
  int cards = 0;
  FILE *config_file = fopen(file_name, "r");

  if (config_file == NULL)
  {
    printf("Error: Cannot open file: %s\n", file_name);
    return 2;
  }

  confirmation = checkMagicNumber(config_file, file_name);
  if (confirmation != 0)
  {
    fclose(config_file);
    return confirmation;
  }

  people = getNumberOfPlayer(config_file);

  Card *current = NULL;

  while (feof(config_file) == 0 && cards <= (people * MAX_CARD_PER_PLAYER))
  {
    Card *new_card = malloc(sizeof(Card));
    if (new_card == NULL)
    {
      printf("Error: Out of memory\n");
      freeCardList(*total_cards);
      fclose(config_file);
      return OUT_OF_MEMORY;
    }

    fscanf(config_file, "%d_%s", &new_card->number_, &new_card->color_);
    new_card->next_ = NULL;

    if (current == NULL)
    {
      *total_cards = new_card;
      current = new_card;
    }
    else
    {
      current->next_ = new_card;
      current = new_card;
    }
    cards++;
  }
  game->amount_of_cards_ = cards;
  game->amount_of_players_ = people;
  game->file_name_ = malloc(sizeof(char) * (strlen(file_name) + 1));
  if (game->file_name_ == NULL)
  {
    printf("Error: Out of memory\n");
    fclose(config_file);
    return OUT_OF_MEMORY;
  }
  strcpy(game->file_name_, file_name);
  fclose(config_file);
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
///
/// Reads the amount of players from file
///
/// @param config_file File pointer
///
/// @return read successfully(number players) or not(-1)
//
int getNumberOfPlayer(FILE *config_file)
{
  int input;
  if (fscanf(config_file, "%d", &input) != 1)
  {
    return -1;
  }
  return input;
}

//---------------------------------------------------------------------------------------------------------------------
///
/// Reads the magic number from file and compares it to the preset one.
///
/// @param config_file File pointer
/// @param file_name string of the entered file name
///
/// @return int correct magic number(0) or not(2 , 3)
//
int checkMagicNumber(FILE *config_file, char *file_name)
{
  char buffer[BUFFER_SIZE];
  if (fgets(buffer, BUFFER_SIZE, config_file) == NULL)
  {
    printf("Error while reading the File");
    return 2;
  }

  if (strcmp(buffer, "ESP\n") != 0)
  {
    printf("Error: Invalid file: %s\n", file_name);
    return 3;
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This function can free any linked list used in my program.
///
/// @param head The pointer to the first card in my linked list.
///
/// @return void
//
void freeCardList(Card *head)
{
  while (head != NULL)
  {
    Card *temp = head;
    head = head->next_;
    free(temp);
  }
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This distributes card to all players. Each player gets 1 Card each Round.
///
/// @param players array of struct Players
/// @param totalCards linked list of all the cards parsed from config file
/// @param game struct Game(holds all important values for the game)
///
/// @return success(0) or error(4)
//
int cardDistribution(Player *players, Card *total_cards, Game *game)
{
  Card *temp = total_cards;
  int people_index = 0;
  int rounds;
  rounds = (game->amount_of_cards_ / game->amount_of_players_) * 2;
  int card_inserted = 0;
  while (temp != NULL && rounds > 0)
  {
    Card *new_card = malloc(sizeof(Card));
    if (new_card == NULL)
    {
      printf("Error: Out of memory\n");
      return OUT_OF_MEMORY;
    }
    new_card->next_ = NULL;
    new_card->number_ = temp->number_;
    new_card->color_ = temp->color_;

    if (card_inserted == rounds)
    {
      people_index = (people_index + 1) % game->amount_of_players_;
    }

    insertCardSorted(&(players[people_index].hand_cards_), new_card);
    temp = temp->next_;
    rounds--;
    card_inserted++;
  }
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This function inserts a struct Card into a already existing linked list. Furthermore the card gets inserted sorted.
///
/// @param HEAD Pointer-Pointer to the first card in linked list.
/// @param new_card pointer to a struct card that should be inserted into the linked list.
///
/// @return void
//
void insertCardSorted(Card **HEAD, Card *new_card)
{
  if (*HEAD == NULL || new_card->number_ <= (*HEAD)->number_)
  {
    new_card->next_ = *HEAD;
    *HEAD = new_card;
  }
  else
  {
    Card *temp = *HEAD;
    while (temp->next_ != NULL && temp->next_->number_ < new_card->number_)
    {
      temp = temp->next_;
    }
    new_card->next_ = temp->next_;
    temp->next_ = new_card;
  }
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This function calls all other functions while the game is running and processes the return values.
///
/// @param players array of struct Player
/// @param game struct Game(holds all important values for the game)
///
/// @return retrun values from previous functions
//
int runningGame(Player *players, Game *game)
{
  int confirmation = 0;

  while (confirmation != 1)
  {
    for (int player_index = 0; player_index < game->amount_of_players_; ++player_index)
    {
      confirmation += checkIfCardsLeft(players[player_index].chosen_cards_, players[player_index].hand_cards_);
    }
    if (confirmation == 0)
    {
      break;
    }
    confirmation = cardChoosingPhase(players, game);
    if (confirmation == 1)
    {
      return 0;
    }
    else if (confirmation == 4)
    {
      return confirmation;
    }
    swapCardDeck(players, game);
    confirmation = actionPhase(players, game);
    if (confirmation == 1)
    {
      return 0;
    }
    else if (confirmation == 4)
    {
      return confirmation;
    }
  }
  printPoints(players, game);
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This function calls opens the config file, calculates points and print functions for the end points.
///
/// @param players array of struct Player
/// @param game struct Game(holds all important values for the game)
///
/// @return void
//
void printPoints(Player *players, Game *game)
{
  int index_longest_row = 0;
  int index_longest_row2 = 0;
  int row_index;
  int total_points;
  int highest_score = 0;
  int counter = 0;
  Card *temp;
  FILE *fp;

  fp = openFile(game->file_name_);

  printf("\n");

  for (int player_index = 0; player_index < game->amount_of_players_; ++player_index)
  {
    row_index = 0;
    total_points = 0;
    temp = players[player_index].row_[row_index];

    while (row_index < MAX_ROW)
    {
      if (temp != NULL)
      {
        counter = 0;
        total_points = calculatePoints(&counter, temp, total_points);

        if (counter > index_longest_row2)
        {
          index_longest_row = row_index;
          index_longest_row2 = counter;
        }
      }
      row_index++;
      temp = players[player_index].row_[row_index];
    }

    updatePlayerPoints(players, player_index, index_longest_row, total_points, &highest_score);
  }

  printResults(players, game, highest_score, fp);

  if (fp == NULL)
  {
    printf("Warning: Results not written to file!\n");
    return;
  }
  fclose(fp);
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This function opens the file it receives.
///
/// @param file_name string of the entered file name
///
/// @return File pointer
//
FILE *openFile(const char *file_name)
{
  FILE *fp = fopen(file_name, "r");
  if (fp != NULL)
  {
    fclose(fp);
    fp = fopen(file_name, "a");
  }
  return fp;
}

//---------------------------------------------------------------------------------------------------------------------
///
/// Checks which player has the highest score and sets the variable in each player struct.
///
/// @param players array of struct Player
/// @param player_index Array-index of player
/// @param index_longest_row Array-index of longest row for each player.
/// @param total_points Total-Points of each player.
/// @param highest_score Highest score in-game.
///
/// @return void
//
void updatePlayerPoints(Player *players, int player_index, int index_longest_row, int total_points, int *highest_score)
{
  players[player_index].player_points_ = calculatePoints(&index_longest_row,
                                                         players[player_index].row_[index_longest_row],
                                                         total_points);

  if (players[player_index].player_points_ > *highest_score)
  {
    *highest_score = players[player_index].player_points_;
  }
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This fucntion calls the print function and write to file function.
///
/// @param players array of struct Player
/// @param highest_score Highest score in-game.
/// @param game struct Game(holds all important values for the game)
/// @param fp File-Pointer
///
/// @return void
//
void printResults(Player *players, Game *game, int highest_score, FILE *fp)
{
  for (int player_index = 1; player_index < game->amount_of_players_; ++player_index)
  {
    if (players[player_index].player_points_ == players[player_index - 1].player_points_)
    {
      printPlayerPoints(player_index, players[player_index - 1].player_points_);
      printPlayerPoints(player_index + 1, players[player_index].player_points_);
      writePlayerPointsToFile(fp, player_index, players[player_index - 1].player_points_);
      writePlayerPointsToFile(fp, player_index + 1, players[player_index].player_points_);
      break;
    }
    else if (players[player_index].player_points_ > players[player_index - 1].player_points_)
    {
      printPlayerPoints(player_index + 1, players[player_index].player_points_);
      writePlayerPointsToFile(fp, player_index + 1, players[player_index].player_points_);
      printPlayerPoints(player_index, players[player_index - 1].player_points_);
      writePlayerPointsToFile(fp, player_index, players[player_index - 1].player_points_);
    }
    else
    {
      printPlayerPoints(player_index, players[player_index - 1].player_points_);
      writePlayerPointsToFile(fp, player_index, players[player_index - 1].player_points_);
      printPlayerPoints(player_index + 1, players[player_index].player_points_);
      writePlayerPointsToFile(fp, player_index + 1, players[player_index].player_points_);
    }
  }

  printf("\n\n");

  for (int player_index = 0; player_index < game->amount_of_players_; ++player_index)
  {
    if (players[player_index].player_points_ == highest_score)
    {
      printf("Congratulations! Player %d wins the game!\n", player_index + 1);
      if(fp != NULL)
      {
        fprintf(fp, "\n\nCongratulations! Player %d wins the game!\n", player_index + 1);
      }
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This function prints the points of the passed player.
///
/// @param player_index Array-index of player.
/// @param points Points of player
///
/// @return void
//
void printPlayerPoints(int player_index, int points)
{
  printf("\nPlayer %d: %d points", player_index, points);
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This function writes the points of the passed player to the file.
///
/// @param fp File-Pointer
/// @param player_index Array-index of player
/// @param points Points of player
///
/// @return void
//
void writePlayerPointsToFile(FILE *fp, int player_index, int points)
{
  if (fp != NULL)
  {
    fprintf(fp, "\nPlayer %d: %d points", player_index, points);
  }
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This Function calculates the points based on the color.
///
/// @param counter how many elements are in a row
/// @param temp current card from row
/// @param total_points Players total points
///
/// @return total_points
//
int calculatePoints(int *counter, Card *temp, int total_points)
{
  while (temp != NULL)
  {
    switch (temp->color_)
    {
      case 'b':
        total_points += BLUE;
        break;
      case 'w':
        total_points += WHITE;
        break;
      case 'g':
        total_points += GREEN;
        break;
      case 'r':
        total_points += RED;
        break;
    }
    temp = temp->next_;
    *counter += 1;
  }
  return total_points;
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This Function is the start and end of card choosing phase.
///
/// @param game struct Game(holds all important values for the game)
/// @param players array of struct Player
///
/// @return success(0) or not(1)
//
int cardChoosingPhase(Player *players, Game *game)
{
  int numbers_entered = 0;
  int error = 0;
  char *input_buffer = NULL;
  int result = ERROR;
  Card *tmp;

  printf("\n-------------------\n"
         "CARD CHOOSING PHASE\n"
         "-------------------\n");

  for (int player_index = 0; player_index < game->amount_of_players_; ++player_index)
  {
    printPlayerStatusInfo(&players[player_index]);

    while (numbers_entered < 2)
    {
      tmp = players[player_index].hand_cards_;
      handleCardChoosingPrompt(numbers_entered, error, player_index);
      if (handleUserInput(&input_buffer, &result, &error, player_index) == 1)
      {
        return 1;
      }

      while (tmp != NULL)
      {
        if (tmp->number_ == atoi(input_buffer))
        {
          handleCardSelection(&players[player_index], &tmp, &numbers_entered, &error);
          break;
        }
        tmp = tmp->next_;
      }

      if (tmp == NULL)
      {
        error = 1;
      }
    }
    numbers_entered = 0;
  }

  free(input_buffer);
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
///
/// Prints correct choosing prompt.
///
/// @param numbers_entered how many cards are already entered
/// @param error is either true(1) or false(0)
/// @param player_index Array-index of player
///
/// @return success(0) or not(1)
//
void handleCardChoosingPrompt(int numbers_entered, int error, int player_index)
{
  if (error == 1)
  {
    printf("Please enter the number of a card in your hand cards!\n"
           "P%d > ", player_index + 1);
  }
  else
  {
    switch (numbers_entered)
    {
      case 0:
        printf("Please choose a first card to keep:\n"
               "P%d > ", player_index + 1);
        break;
      case 1:
        printf("Please choose a second card to keep:\n"
               "P%d > ", player_index + 1);
        break;
      default:
        break;
    }
  }
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This Function reads the user input and handles it accordingly
///
/// @param game struct Game(holds all important values for the game)
/// @param players array of struct Player
///
/// @return success(0) or not(1)
//
int handleUserInput(char **input_buffer, int *result, int *error, int player_index)
{
  while (*result == ERROR)
  {
    userInput(input_buffer);
    *result = cardChosingPhaseCommands(*input_buffer);
    if (*result == 1)
    {
      free(*input_buffer);
      return 1;
    }
    else if (*result == ERROR)
    {
      printf("P%d > ", player_index + 1);
      *error = 1;
    }
  }
  *result = ERROR;
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This Function reads the user input and handles it accordingly
///
/// @param player struct player
/// @param tmp Pointer-Pointer for selected Card
/// @param numbers_entered pointer to numbers entered: keeps track of how many card are entered already
/// @param error pointer to int error: keep track if a error occured and changes value
///
/// @return success(0) or not(4)
//
int handleCardSelection(Player *player, Card **tmp, int *numbers_entered, int *error)
{
  Card *new_card = malloc(sizeof(Card));
  if (new_card == NULL)
  {
    printf("Error: Out of memory\n");
    return OUT_OF_MEMORY;
  }

  copyCardData(*tmp, new_card);
  insertCardSorted(&player->chosen_cards_, new_card);
  removeCardFromHand(&player->hand_cards_, *tmp);
  (*numbers_entered)++;
  *error = 0;
  return 0;
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This Function copies the data of one card and creates a new struct at the destination pointer.
///
/// @param source pointer of card to be copied
/// @param destination pointer of destination
///
/// @return void
//
void copyCardData(Card *source, Card *destination)
{
  if (source != NULL && destination != NULL)
  {
    destination->number_ = source->number_;
    destination->color_ = source->color_;
    destination->next_ = NULL;
  }
}

//---------------------------------------------------------------------------------------------------------------------
///
/// This function reads user input from standard input, dynamically allocates memory to store the input, and returns the
/// size of the input string (excluding the null terminator).
///
/// @param user_input Pointer to a pointer that will store the user input string. Should be NULL or point to allocated
///                   memory that can be reallocated if needed.
///
/// @return If successful, returns the size of the input string (excluding null terminator); otherwise, returns ERROR.
///
size_t userInput(char **user_input)
{
  size_t current_size = sizeof(*user_input) / 8;
  size_t size = BUFFER_SIZE;
  size_t i = 0;
  int ch;

  if (*user_input == NULL)
  {
    *user_input = (char *) malloc(size);
    current_size = size;
    if (*user_input == NULL)
    {
      return ERROR;
    }
  }
  while ((ch = fgetc(stdin)) != EOF && ch != '\n')
  {
    if (i >= current_size - 1)
    {
      current_size *= 2;
      char *temp = realloc(*user_input, current_size);
      if (temp == NULL)
      {
        return ERROR;
      }
      *user_input = temp;
    }
    (*user_input)[i++] = (char) ch;
  }
  if (i == 0 && ch == EOF)
  {
    return ERROR;
  }
  char *temp = realloc(*user_input, i + 1);
  if (temp == NULL)
  {
    return ERROR;
  }
  *user_input = temp;
  (*user_input)[i] = '\0';
  return i;
}

//------------------------------------------------------------------------------------------------------------------------------------
///
/// This function removes a specified card from a linked list of cards.
///
/// @param HEAD Pointer to the head of the linked list.
/// @param hand_card Pointer to the card to be removed.
///
/// @return void
///
void removeCardFromHand(struct _Card_ **HEAD, Card *hand_card)
{
  Card *temp = *HEAD;
  Card *previous = NULL;
  while (temp != NULL)
  {
    if (hand_card == temp)
    {
      if (previous == NULL)
      {
        *HEAD = temp->next_;
        free(hand_card);
        return;
      }
      else
      {
        previous->next_ = temp->next_;
        free(hand_card);
        return;
      }
    }
    previous = temp;
    temp = temp->next_;
  }
}

//----------------------------------------------------------------------------------------------------------------------
///
/// This function swaps the hand card decks among players after the card choosing phase is over.
///
/// @param players An array of Player structures representing the players in the game.
/// @param game A pointer to the Game structure containing game-related information.
///
/// @return void
///
void swapCardDeck(Player *players, Game *game)
{
  printf("\n"
         "Card choosing phase is over - passing remaining hand cards to the next player!\n"
         "\n");
  Card *head_of_previous_deck;
  Card *tmp;
  for (int player_index = 0; player_index < game->amount_of_players_; ++player_index)
  {
    if (player_index == 0)
    {
      head_of_previous_deck = players[player_index].hand_cards_;
      continue;
    }
    tmp = players[player_index].hand_cards_;
    players[player_index].hand_cards_ = head_of_previous_deck;
    players[player_index - 1].hand_cards_ = tmp;
  }
}

//----------------------------------------------------------------------------------------------------------------------
///
/// This function represents the action phase of the game, where players take turns performing actions with their chosen
/// cards.
///
/// @param players An array of Player structures representing the players in the game.
/// @param game A pointer to the Game structure containing game-related information.
///
/// @return 0 if the action phase completes successfully, 1 if a player chooses to exit the game, or an error code
/// otherwise.
///
int actionPhase(Player *players, Game *game)
{
  char *input_buffer = NULL;
  int result;
  printf("------------\n"
         "ACTION PHASE\n"
         "------------\n");
  for (int player_index = 0; player_index < game->amount_of_players_; ++player_index)
  {
    for (int card_index = 0; card_index < 2; ++card_index)
    {
      while (checkIfCardsLeft(players[player_index].chosen_cards_, NULL) == 1)
      {
        printPlayerStatusInfo(&players[player_index]);
        printf("What do you want to do?\n"
               "P%d > ", player_index + 1);
        do
        {
          userInput(&input_buffer);
          result = actionPhaseCommands(input_buffer, &players[player_index]);
          if (result == 1)
          {
            free(input_buffer);
            return result;
          }
          else if (result == ERROR)
          {
            printf("P%d > ", player_index + 1);
          }
          else if (result == OUT_OF_MEMORY)
          {
            free(input_buffer);
            return result;
          }
        } while (result == ERROR);
      }
    }
    printPlayerStatusInfo(&players[player_index]);
  }
  printf("\n"
         "Action phase is over - starting next game round!\n");
  free(input_buffer);
  return 0;
}

//----------------------------------------------------------------------------------------------------------------------
///
/// This function checks if there are any cards left in the specified card lists.
///
/// @param chosen_cards Pointer to the head of the chosen cards linked list.
/// @param hand_cards Pointer to the head of the hand cards linked list.
///
/// @return 1 if there are cards left in either list, 0 otherwise.
///
int checkIfCardsLeft(struct _Card_ *chosen_cards, struct _Card_ *hand_cards)
{
  if (chosen_cards != NULL)
  {
    return 1;
  }
  if (hand_cards != NULL)
  {
    return 1;
  }
  return 0;
}

//----------------------------------------------------------------------------------------------------------------------
///
/// This function prints the status information of a player, including their hand cards, chosen cards, and rows.
///
/// @param players Pointer to the Player structure representing the player.
///
/// @return void
///
void printPlayerStatusInfo(Player *players)
{
  printf("\n"
         "Player %d:\n"
         "  hand cards:", players->index + 1);
  Card *current_card = players->hand_cards_;
  while (current_card != NULL)
  {
    printf(" %d_%c", current_card->number_, current_card->color_);
    current_card = current_card->next_;
  }
  printf("\n");
  printf("  chosen cards:");
  current_card = players->chosen_cards_;
  while (current_card != NULL)
  {
    printf(" %d_%c", current_card->number_, current_card->color_);
    current_card = current_card->next_;
  }
  for (int row_index = 0; row_index < MAX_ROW; ++row_index)
  {
    current_card = players->row_[row_index];
    if (current_card != NULL)
    {
      printf("\n  row_%d:", row_index + 1);
    }
    while (current_card != NULL)
    {
      printf(" %d_%c", current_card->number_, current_card->color_);
      current_card = current_card->next_;
    }
  }
  printf("\n\n");
}

//----------------------------------------------------------------------------------------------------------------------
///
/// This function processes the user command during the action phase and performs the corresponding action.
///
/// @param string The user input string containing the command.
/// @param player Pointer to the Player structure representing the current player.
///
/// @return 0 on successful command execution, ERROR on invalid command, or an error code for other cases.
///
int actionPhaseCommands(char *string, Player *player)
{
  char *token = strtok(string, " ");

  if (token == NULL)
  {
    printf("Please enter a valid command!\n");
    return ERROR;
  }

  if (stringCompareCaseInsensitive(token, "help") == 0)
  {
    return handleHelpCommand();
  }
  else if (stringCompareCaseInsensitive(token, "place") == 0)
  {
    return handlePlaceCommand(player);
  }
  else if (stringCompareCaseInsensitive(token, "discard") == 0)
  {
    return handleDiscardCommand(player);
  }
  else if (stringCompareCaseInsensitive(token, "quit") == 0)
  {
    return handleQuitCommand();
  }
  else
  {
    printf("Please enter a valid command!\n");
    return ERROR;
  }
}

int handleHelpCommand(void)
{
  char *token = strtok(NULL, "");

  if (token != NULL)
  {
    printf("Please enter the correct number of parameters!\n");
    return ERROR;
  }
  else
  {
    printf("\nAvailable commands:\n"
           "\n"
           "- help\n"
           "  Display this help message.\n"
           "\n"
           "- place <row number> <card number>\n"
           "  Append a card to the chosen row or if the chosen row does not exist create it.\n"
           "\n"
           "- discard <card number>\n"
           "  Discard a card from the chosen cards.\n"
           "\n"
           "- quit\n"
           "  Terminate the program.\n"
           "\n");
    return 0;
  }
}

//----------------------------------------------------------------------------------------------------------------------
///
/// This function handles the "help" command, providing information about available commands or
/// displaying an error message
/// if incorrect parameters are provided.
///
/// @return 0 on successful help display, ERROR on incorrect parameters.
///
int handlePlaceCommand(Player *player)
{
  int row, cardNumber;
  char *row_token;
  char *token;
  char *endptr;
  row_token = strtok(NULL, " ");

  if (row_token == NULL)
  {
    printf("Please enter the correct number of parameters!\n");
    return ERROR;
  }
  token = strtok(NULL, " ");
  if (token == NULL)
  {
    printf("Please enter the correct number of parameters!\n");
    return ERROR;
  }

  row = (int) strtol(row_token, &endptr, 10);

  if (*endptr != '\0')
  {
    printf("Please enter a valid row number!\n");
    return ERROR;
  }
  if (checkOutOfBound(row) == ERROR)
  {
    return ERROR;
  }

  cardNumber = (int) strtol(token, &endptr, 10);

  if (*endptr != '\0')
  {
    printf("Please enter the number of a card in your hand cards!\n");
    return ERROR;
  }

  token = strtok(NULL, " ");
  if (token != NULL)
  {
    printf("Please enter the correct number of parameters!\n");
    return ERROR;
  }
  return placeCardInRow(row - 1, cardNumber, player);
}

//----------------------------------------------------------------------------------------------------------------------
///
/// This function handles the "discard" command, validating parameters and calling the discardCard function accordingly.
///
/// @param player Pointer to the Player structure representing the current player.
///
/// @return The result of the discardCard function or an error code if parameters are incorrect.
///
int handleDiscardCommand(Player *player)
{
  char *token;
  char *number = strtok(NULL, " ");
  token = strtok(NULL, " ");

  if (token != NULL || number == NULL)
  {
    printf("Please enter the correct number of parameters!\n");
    return ERROR;
  }

  return discardCard(number, player);
}

//----------------------------------------------------------------------------------------------------------------------
///
/// This function handles the "quit" command, validating parameters and signaling the program termination.
///
/// @return 1 to signal program termination or ERROR if incorrect parameters are provided.
///
int handleQuitCommand(void)
{
  char *token = strtok(NULL, " ");

  if (token != NULL)
  {
    printf("Please enter the correct number of parameters!\n");
    return ERROR;
  }
  return 1;
}

//----------------------------------------------------------------------------------------------------------------------
///
/// This function discards a card from the chosen cards of a player, based on the provided card number.
///
/// @param strtok The string token representing the card number to be discarded.
/// @param player Pointer to the Player structure representing the current player.
///
/// @return 0 on successful card discard, ERROR if player or chosen cards are NULL,
/// or an error message if the card number is not found in the chosen cards.
///
int discardCard(char *strtok, Player *player)
{
  char *ptr;
  if (player == NULL)
  {
    return ERROR;
  }
  if (player->chosen_cards_ == NULL)
  {
    return ERROR;
  }

  Card *head = player->chosen_cards_;
  long number = strtol(strtok, &ptr, 10);

  while (head != NULL)
  {
    if (number == head->number_)
    {
      removeCardFromHand(&player->chosen_cards_, head);
      return 0;
    }
    head = head->next_;
  }
  printf("Please enter the number of a card in your chosen cards!\n");
  return ERROR;
}

//----------------------------------------------------------------------------------------------------------------------
///
/// This function checks if the provided row number is within the valid range.
///
/// @param row The row number to be checked.
///
/// @return 0 if the row number is within the valid range, or ERROR with an error message if out of bounds.
///
int checkOutOfBound(int row)
{
  if (row < MIN_ROW || row > MAX_ROW)
  {
    printf("Please enter a valid row number!\n");
    return ERROR;
  }
  return 0;
}

//----------------------------------------------------------------------------------------------------------------------
///
/// This function places a card in the specified row of a player's board, based on the provided card number.
///
/// @param row The row number where the card should be placed.
/// @param number The card number to be placed in the row.
/// @param player Pointer to the Player structure representing the current player.
///
/// @return 0 on successful card placement, ERROR if out of memory or invalid card number,
/// or an error message if the card cannot be placed in the chosen row.
///
int placeCardInRow(int row, int number, Player *player)
{
  Card *new_card = malloc(sizeof(Card));
  if (new_card == NULL)
  {
    printf("Error: Out of memory\n");
    return OUT_OF_MEMORY;
  }
  Card *head = player->chosen_cards_;
  Card *temp;
  Card *head_row = player->row_[row];

  while (head != NULL)
  {
    if (head->number_ == number)
    {
      new_card->number_ = head->number_;
      new_card->color_ = head->color_;
      new_card->next_ = NULL;

      if (player->row_[row] == NULL)
      {
        player->row_[row] = new_card;
        removeCardFromHand(&player->chosen_cards_, head);
        return 0;
      }
      else
      {
        if (head->number_ < player->row_[row]->number_)
        {
          temp = player->row_[row];
          player->row_[row] = new_card;
          player->row_[row]->next_ = temp;
          removeCardFromHand(&player->chosen_cards_, head);
          return 0;
        }
        while (head_row->next_ != NULL)
        {
          head_row = head_row->next_;
        }
        if (head->number_ > head_row->number_)
        {
          head_row->next_ = new_card;
          removeCardFromHand(&player->chosen_cards_, head);
          return 0;
        }
      }
      printf("This card cannot extend the chosen row!\n");
      free(new_card);
      return ERROR;
    }
    head = head->next_;
  }
  printf("Please enter the number of a card in your chosen cards!\n");
  free(new_card);
  return ERROR;
}

//----------------------------------------------------------------------------------------------------------------------
///
/// This function processes the user command during the card choosing phase, validates parameters, and returns the
/// card number to be chosen or signals program termination.
///
/// @param string The user input string containing the command.
///
/// @return The card number to be chosen (if a valid card number is provided), 1 to signal program termination, or
///         ERROR with an error message if parameters are incorrect.
///
int cardChosingPhaseCommands(char *string)
{
  char *token = strtok(string, " ");

  if (token == NULL)
  {
    printf("Please enter the number of a card in your hand cards!\n");
    return ERROR;
  }
  if (strcasecmp(token, "quit") == 0)
  {
    token = strtok(NULL, " ");
    if (token != NULL)
    {
      printf("Please enter the correct number of parameters!\n");
      return ERROR;
    }
    return 1;
  }
  else
  {
    char *endptr;
    long number = strtol(token, &endptr, 10);

    if (*endptr != '\0')
    {
      printf("Please enter the number of a card in your hand cards!\n");
      return ERROR;
    }

    token = strtok(NULL, " ");
    if (token != NULL)
    {
      printf("Please enter the number of a card in your hand cards!\n");
      return ERROR;
    }

    return (int) number;
  }
}

//----------------------------------------------------------------------------------------------------------------------
///
/// This function compares two strings in a case-insensitive manner.
///
/// @param string1 The first string to be compared.
/// @param string2 The second string to be compared.
///
/// @return 0 if the strings are equal (case-insensitive), 1 otherwise.
///
int stringCompareCaseInsensitive(const char *string1, const char *string2)
{
  while (*string1 && (*string1 == *string2 || tolower(*string1) == tolower(*string2)))
  {
    string1++;
    string2++;
  }

  return (*string1 == *string2) ? 0 : 1;
}
