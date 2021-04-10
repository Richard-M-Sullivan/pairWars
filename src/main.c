#include <stdio.h> 
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

/////////////////////////////////////////////

//CardQueue implamentation
//the queue will represent a 52 card deck, and will hold 52 cards
#define NUM_OF_CARDS 52
typedef struct CardQueue{
    int top;
    int bottom;
    int queue[NUM_OF_CARDS];
}CardQueue;

void CardQueue_init(CardQueue* queue);
void CardQueue_push(CardQueue* queue, int new_card);
int CardQueue_pop( CardQueue* queue);
void CardQueue_shuffle(CardQueue* queue);
void CardQueue_print(CardQueue* queue);
void CardQueue_str(char str[],CardQueue* queue);

///////////////////////////////////////////

//player functions
#define NUM_OF_PLAYERS 3
#define HAND_SIZE 2
void players_print(int players[NUM_OF_PLAYERS][HAND_SIZE]);

///////////////////////////////////////////

//dealer thread functions and data
typedef struct DealerInfo{
    int* playerTurnPtr;
    CardQueue* deckPtr;
    int (*playerCards)[NUM_OF_PLAYERS][HAND_SIZE];
    int* playerDiscardSelectionPtr;
    int* playerStatusPtr;
}DealerInfo;

void* dealerThread(void* vargp){
    
    bool firstHand = true;
    int nextPlayer = 0;
    int currentPlayer = 0;

    //work continuously
    while(true){
        //if a player has won
        if( *(((DealerInfo*)vargp)->playerStatusPtr) == 1 ){
            break;
        }
        //if it is the first hand the deck needs to be shuffled and the initial
        //cards need to be given to each player as well as the first players second card
        else if( firstHand == true && *(((DealerInfo*)vargp)->playerTurnPtr) == -1 ){
            printf("initial deal\n");

            CardQueue_shuffle(((DealerInfo*)vargp)->deckPtr);
    
            for(int i=0; i<NUM_OF_PLAYERS; i++){
                (*((DealerInfo*)vargp)->playerCards)[i][0] = CardQueue_pop( ((DealerInfo*)vargp)->deckPtr );
            }

            (*((DealerInfo*)vargp)->playerCards)[nextPlayer][1] = CardQueue_pop( ((DealerInfo*)vargp)->deckPtr );
            nextPlayer ++; 

            *(((DealerInfo*)vargp)->playerTurnPtr) = currentPlayer;

            firstHand = false;
        }
        //if it is the dealers turn, then check current players status and card to return,
        // then give the next player their card and let them know it is their turn to play
        else if( *(((DealerInfo*)vargp)->playerTurnPtr) == -1 ){
            printf("evaluating card and passing next player card\n");

            //if the current player didn't win
            if(*(((DealerInfo*)vargp)->playerStatusPtr) == 0){
                //checkout current player discard
                //put their card back in the deck and organize their hand
                if( *(((DealerInfo*)vargp)->playerDiscardSelectionPtr) == 0){
                   CardQueue_push( ((DealerInfo*)vargp)->deckPtr, (*((DealerInfo*)vargp)->playerCards)[currentPlayer][0]);
                   (*((DealerInfo*)vargp)->playerCards)[currentPlayer][0] = (*((DealerInfo*)vargp)->playerCards)[currentPlayer][1];
                   (*((DealerInfo*)vargp)->playerCards)[currentPlayer][1] = 0;
                }
                else{
                   CardQueue_push( ((DealerInfo*)vargp)->deckPtr, (*((DealerInfo*)vargp)->playerCards)[currentPlayer][1]);
                   (*((DealerInfo*)vargp)->playerCards)[currentPlayer][1] = 0;

                }

                //give the next player their card
                (*((DealerInfo*)vargp)->playerCards)[nextPlayer][1] = CardQueue_pop( ((DealerInfo*)vargp)->deckPtr );

                //alert the player that they are allowed to play
                *(((DealerInfo*)vargp)->playerTurnPtr) = nextPlayer;

                //update next and current player
                currentPlayer = nextPlayer;
                nextPlayer = (nextPlayer+1) % NUM_OF_PLAYERS;
            }
            else{
                break;
            }
        }
    }
    return 0;
}

///////////////////////////////////////////////

//player thread functions and data

////////////////////////////////////////////////

typedef struct PlayerInfo{
    int playerNum;
    int* playerTurnPtr;
    int* playerStatusPtr;
    int* playerDiscardSelectionPtr;
    int* card1;
    int* card2;
}PlayerInfo;

void* playerThread(void* vargp){

    while(true){
        //check if anyone has won
        if( *(((PlayerInfo*)vargp)->playerStatusPtr) == 1){
            break;
        }
        //check if it is your turn
        if( *(((PlayerInfo*)vargp)->playerTurnPtr) == ((PlayerInfo*)vargp)->playerNum){
            //if so print your name
            printf("I am a player thread: %d\n",((PlayerInfo*)vargp)->playerNum);

            //if the two cards are the same then say that you won
            if( *(((PlayerInfo*)vargp)->card1) == *(((PlayerInfo*)vargp)->card2)){
               *(((PlayerInfo*)vargp)->playerStatusPtr) = 1;
            }
            //if the cards are not alike
            else{
                //pick a random card (1 or 2) and
                //let the dealer know what your choice for a discard is
                *(((PlayerInfo*)vargp)->playerDiscardSelectionPtr) = rand() % 2;
            }
            //tell the dealer that it is his turn
            *(((PlayerInfo*)vargp)->playerTurnPtr) = -1;
        }
    }

    return 0;
}

  //////////////////////
 // Main Application //
//////////////////////

int main() { 
    //initialize random number generator
    srand(time(NULL));

      /////////////////
     // Setup Table //
    /////////////////

    /*
    The table reffers to the data that the players and dealer threads will 
     be able to access and use as a means of communication.
    */

    //create a deck of cards
    CardQueue deck;

    //create card holders for players to use
    int cards[NUM_OF_PLAYERS][HAND_SIZE] = {0};

    //create a turn indicators
    int playerTurn; //indicates which player is allowed to play, -1 = dealer turn
    int playerDiscardSelection;//indicates which card the player wants to discard
    int playerStatus;// 0 = not win, and 1 = win
    
     ///////////////////////////////
    // Create Dealer and Players //
   ///////////////////////////////

    //create dealer thread
    pthread_t dealer;

    //set dealer information
    DealerInfo dealerinfo;
    dealerinfo.playerTurnPtr = &playerTurn;
    dealerinfo.deckPtr = &deck;
    dealerinfo.playerCards = &cards;
    dealerinfo.playerDiscardSelectionPtr = &playerDiscardSelection;
    dealerinfo.playerStatusPtr = &playerStatus;

    //create array of player threads
    pthread_t players[NUM_OF_PLAYERS];

    //set player information
    PlayerInfo playerInfo[3];
    for(int i=0; i<NUM_OF_PLAYERS; i++){
        playerInfo[i].playerNum = i;
        playerInfo[i].playerTurnPtr = &playerTurn;
        playerInfo[i].playerStatusPtr = &playerStatus;
        playerInfo[i].playerDiscardSelectionPtr = &playerDiscardSelection;
        playerInfo[i].card1 = &cards[i][0];
        playerInfo[i].card2 = &cards[i][1];
    }

     /////////////////
    // Start Round //
   /////////////////

    for(int i=0; i<3; i++){

         ///////////////////////////
        // Resetting Game Values //
       ///////////////////////////
        playerTurn = -1;
        playerDiscardSelection = 0;
        playerStatus = 0;
        CardQueue_init(&deck);

         ///////////////////
        // Launch Dealer //
       ///////////////////
       
        /*
        In this stage the dealer thread will be created. He will see that it is no
        players turn, and so he will deal out the initial cards and wait for people
        to show up and play. (he hopes they arrive soon for he hates to be alone.
        */
    
        //launch dealer thread with information
        pthread_create(&dealer,NULL,dealerThread,&dealerinfo);

          ////////////////////
         // Launch Players //
        ////////////////////

        /*
        Here the players join the dealer and start the game. They all look at
        the player turn indicator and use that to decide whose turn it is to 
        play. The players all will make it the dealers turn after they make
        all of their decisions.
        */

        //launch players thread with information
        for(int i=0; i<NUM_OF_PLAYERS; i++){
            pthread_create(&players[i],NULL,playerThread,&playerInfo[i]); 
        }

        //join the dealer and players
        pthread_join(dealer,NULL);

        for(int i=0; i<NUM_OF_PLAYERS; i++){
            pthread_join(players[i],NULL);
        }

        //print cards
        for(int i=0; i<NUM_OF_PLAYERS; i++){
            for(int j=0; j<HAND_SIZE; j++){
                printf("%d ", cards[i][j]);
            }
            printf("\n");
        }

    }// end outer for loop

    return 0; 

}// end main funciton

//sets the card queue to be a full unshuffled deck of cards 1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4, ...
void CardQueue_init(CardQueue* queue){
    int counter = 0;

    //for the numbers 1 - 13 put them into the array 4 times each
    for(int i=0;i<13;i++){
        for(int j=0;j<4;j++){
            queue->queue[counter] = i+1;
            counter++;
        }
    }
    //set the top and bottom of the queue to appropriate initial positions
    queue->top = 0;
    queue->bottom = NUM_OF_CARDS - 1;
}

void CardQueue_push(CardQueue* queue,int new_card){
    //if the queue is empty
    if (queue->top == -1){
        //put in a card at the bottom and set top and bottom to each other
        queue->top = 0;
        queue->bottom = 0;
        queue->queue[queue->bottom] = new_card;
    }
    //if queue is not full
    else if ((queue->bottom + 1) % NUM_OF_CARDS != queue->top ){
        //incrament the bottom and put in a card
        queue->bottom = (queue->bottom + 1)% NUM_OF_CARDS;
        queue->queue[queue->bottom] = new_card;
    }
    //if the queue is full
    else{
        printf("error queue full\n");
    }
}

int CardQueue_pop( CardQueue* queue){
    int result = 0;
    //if the queue is empty
    if (queue->top == -1){
        printf("error queue is empty");
    }
    //if there is one card in the queue
    else if(queue->top == queue->bottom){
        //store the card in the top then make top and bottom indicate queue empty
        result = queue->queue[queue->top];
        queue->top = -1;
        queue->bottom = -1;
    }
    //if there is more than on card in the queue
    else{
        //store the card on top and incrament the top to the next card
        result = queue->queue[queue->top];
        queue->top = (queue->top+1) % NUM_OF_CARDS;
    }
    return result;
}

//shuffles the deck (only works on a full deck, otherwise will print an error message)
void CardQueue_shuffle(CardQueue* queue){
    //check if the deck is full
    if((queue->bottom+1) % NUM_OF_CARDS == queue->top){
        int index = 0;
        int temp = 0;
        
        //generate a random number within the range of 0 and the end of the array - i
        //then swap that item with the end of the array - i
        //this will build a list from the back to the front of randomly selected numbers
        // , thus putting the array into a random order
        for(int i= NUM_OF_CARDS -1; i>0; i--){
            index = rand() % (i+1);
            temp = queue->queue[index];
            queue->queue[index] = queue->queue[i];
            queue->queue[i] = temp;
        }
    }
    //if array not full print error
    else{
        printf("error array not full. try CardQueue_init(CardQueue* queue)");
    }
}

//prints the card queue to the console (formatted to print 12 cards per row)
void CardQueue_print(CardQueue* queue){
    int index = queue->top;
    int i = queue->bottom - queue->top;
    int counter = 0;

    i = (i<0) ? (NUM_OF_CARDS + i) : i ;

    for(   ;i >= 0; i-- ){
        printf("%d ",queue->queue[index]);
        index = (index+1) % NUM_OF_CARDS;
        counter++;
        if(counter == 12){
            printf("\n");
            counter = 0;
        }
    }
    printf("\n\n");
}

//places the cards in the card que inside of a given string. 
//(The list items are seperated by spaces)
void CardQueue_str(char str[],CardQueue* queue){
    str[0] = 0;

    int index = queue->top;

    //find the distance between the top and bottom
    int i = queue->bottom - queue->top;
    i = (i<0) ? (NUM_OF_CARDS + i) : i ;

    //loop that number of times moving the index from queue->top to queue->bottom
    for(   ;i >= 0; i-- ){
        //counter keeps track of the number of digits and is used for positioning in the string
        sprintf(str,"%s%d ",str,queue->queue[index] );
        index = (index+1) % NUM_OF_CARDS;
    }
}

//prints each players cards and if they won or not to the console
void players_print(int players[NUM_OF_PLAYERS][HAND_SIZE]){
    for(int i=0; i<NUM_OF_PLAYERS; i++){
        printf("PLAYER %d\nHAND: %d, %d\n",i+1,players[i][0],players[i][1]);
        printf("WIN: %s\n\n", (players[i][0] == players[i][1]) ? "YES":"NO");
    }
}
