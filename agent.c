/*********************************************************
 *  agent.c
 *  Nine-Board Tic-Tac-Toe Agent
 *  COMP3411/9414/9814 Artificial Intelligence
 *  Dion Earle, Assignment 3
 */

/*
Briefly describe how your program works, including any algorithms and data structures
employed, and explain any design decisions you made along the way.

When my agent is asked what position on the current board it wants to play at, whether that
be through the second_move, third_move or next_move commands, it utilises the alpha-beta
search algorithm, specifically the negamax version. I decided to use alpha-beta search
over regular minimax as it allowed the search depth to be nearly doubled whilst maintaining
the same overall speed, and using the negamax formulation of this simply allowed the code to be
condensed and easier to follow. In implementing this alpha-beta search, I chose to utilise a
helper function for the first iteration that would work in the same way as the main search
function, yet also sets up some initial values for the search and returns the selected move
rather than a value for alpha. Doing so simplified the code dramatically, since some steps such
as checking for a terminal node or zero depth aren't necessary in the first iteration, and it
allowed for the position that gave the maximum alpha to be easily selected.

At the beginning of each iteration of the alpha-beta search, there are two checks to be made before
carrying on. The first is determing whether the current node is terminal, meaning either the
opponent has three in a row, or the square is filled, with these situations indicating a loss and a
draw respectively. A seperate function was used to evaluate this, which simply uses brute force to
check every row, column and diagonal of all squares. The second thing to note is when the depth
equals zero, with this stopping the search from going any deeper and instead returning the heuristic
value of this node. A very tough choice I had to make was deciding what heuristic function I would
use. I decided to use a variant of the regular tic-tac-toe heuristic described in the tutorial,
being 3*X2 + X1 - (3*O2 + O1). This value is calculated for each square of the grid using a seperate
function, with the final heuristic for the node being the sum of all such values. Finally, when
testing my agent against different opponents I had to decide what search depth was most appropriate
to use. Whilst lower depths made calculations quicker, higher depths provided better solutions. I
ended up selecting a depth of 10 for my search, and whilst I would have preferred to increase this
further, doing so made the program run very close to being outside of the provided time limit,
and I decided it wasn't worth the risk of losing because of a timeout.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "common.h"
#include "agent.h"
#include "game.h"

#define MAX_MOVE 81

int board[10][10];
int move[MAX_MOVE+1];
int player;
int m;

/*********************************************************//*
   Print usage information and exit
*/
void usage( char argv0[] )
{
  printf("Usage: %s\n",argv0);
  printf("       [-p port]\n"); // tcp port
  printf("       [-h host]\n"); // tcp host
  exit(1);
}

/*********************************************************//*
   Parse command-line arguments
*/
void agent_parse_args( int argc, char *argv[] )
{
  int i=1;
  while( i < argc ) {
    if( strcmp( argv[i], "-p" ) == 0 ) {
      if( i+1 >= argc ) {
        usage( argv[0] );
      }
      port = atoi(argv[i+1]);
      i += 2;
    }
    else if( strcmp( argv[i], "-h" ) == 0 ) {
      if( i+1 >= argc ) {
        usage( argv[0] );
      }
      host = argv[i+1];
      i += 2;
    }
    else {
      usage( argv[0] );
    }
  }
}

/*********************************************************//*
   Called at the beginning of a series of games
*/
void agent_init()
{
  struct timeval tp;

  // generate a new random seed each time
  gettimeofday( &tp, NULL );
  srandom(( unsigned int )( tp.tv_usec ));
}

/*********************************************************//*
   Called at the beginning of each game
*/
void agent_start( int this_player )
{
  reset_board( board );
  m = 0;
  move[m] = 0;
  player = this_player;
}

/*********************************************************//*
   Choose second move and return it
*/
int agent_second_move( int board_num, int prev_move )
{
  // Based on the information passed in through the arguments, we update the move list
  // and the positions played on the board.
  int this_move;
  move[0] = board_num;
  move[1] = prev_move;
  board[board_num][prev_move] = !player;
  m = 2;

  // We then use the function setup_search to begin the alpha-beta search, with the final
  // returned move from this search being assigned to this_move.
  this_move = setup_search(prev_move);

  // Finally, based on the move selected above, we update the move list and place this
  // selection on the board.
  move[m] = this_move;
  board[prev_move][this_move] = player;
  return( this_move );
}

/*********************************************************//*
   Choose third move and return it
*/
int agent_third_move(
                     int board_num,
                     int first_move,
                     int prev_move
                    )
{
  // Based on the information passed in through the arguments, we update the move list
  // and the positions played on the board.
  int this_move;
  move[0] = board_num;
  move[1] = first_move;
  move[2] = prev_move;
  board[board_num][first_move] =  player;
  board[first_move][prev_move] = !player;
  m=3;

  // We then use the function setup_search to begin the alpha-beta search, with the final
  // returned move from this search being assigned to this_move.
  this_move = setup_search(prev_move);

  // Finally, based on the move selected above, we update the move list and place this
  // selection on the board.
  move[m] = this_move;
  board[move[m-1]][this_move] = player;
  return( this_move );
}

/*********************************************************//*
   Choose next move and return it
*/
int agent_next_move( int prev_move )
{
  // Based on the information passed in through the arguments, we update the move list
  // and the positions played on the board.
  int this_move;
  m++;
  move[m] = prev_move;
  board[move[m-1]][move[m]] = !player;
  m++;

  // We then use the function setup_search to begin the alpha-beta search, with the final
  // returned move from this search being assigned to this_move.
  this_move = setup_search(prev_move);

  // Finally, based on the move selected above, we update the move list and place this
  // selection on the board.
  move[m] = this_move;
  board[move[m-1]][this_move] = player;
  return( this_move );
}

/*********************************************************//*
   This is the first iteration of the alpha-beta search, returning the position to play in
*/
int setup_search( int current_board )
{

  // This is the depth in which our alpha-beta search will traverse down to.
  // Higher depths will provide better results yet will take more time to compute.
  int depth = 10;

  // When we start our alpha-beta search, we set alpha = -infinity and beta = infinity.
  // Since the maximum heuristic value for any node is 100, using -200 and 200 will suffice.
  int alpha = -200;
  int beta = 200;

  // The first iteration of our alpha-beta search is done here so we can determine the actual move
  // we want to make, as our alpha_beta_search function only returns the value of alpha not the move
  // which provided this value.
  int this_move = -1;

  // We loop through for all possible positions on the current board
  int i;
  for (i = 1; i <= 9; ++i) {

    // We first check if this position is already filled on the board.
    // If it is, playing here would be an illegal move, so we ignore this position and move on.
    if (board[current_board][i] == EMPTY) {

      // For the chosen position, we assign this move on the board.
      board[current_board][i] = player;

      // We now call our alpha_beta_search function to recursively check all children nodes,
      // either until its terminal or the depth is reached. The depth is decreased by 1 and
      // the new board will be the position we are playing on the current board, being i.
      // Note since we are using the negamax variant, our value of alpha is -beta
      // and our value of beta is -alpha. Also, it is considered from the perspective
      // of the opponent, so we pass in !player as the current player.
      int search_result = -alpha_beta_search(i, depth - 1, -beta, -alpha, !player);

      // After attaining our results from the search, we can undo our move on this position.
      board[current_board][i] = EMPTY;

      // Here we are taking the max of our current alpha and the return value
      // of the alpha beta search, assigning this as alpha.
      if (search_result > alpha) {
        alpha = search_result;

        // If the alpha beta search returned a larger alpha than our previous alpha,
        // we not only update alpha but also update the move to be chosen.
        this_move = i;
      }
    }
  }

  // We return the chosen move after the search is completed.
  return this_move;
}

/*********************************************************//*
   Negamax formulation of alpha-beta search
*/
int alpha_beta_search( int current_board, int depth, int alpha, int beta, int current_player )
{

  // Before we continue with the search, we first check if the current node is terminal.
  // This involves checking all columns, rows and diagonals to see if the opponent
  // got 3 in a row in the previous move, in which case we return -100, or if there
  // are any squares that are filled so that a move can no longer be made, where 0 is returned.
  int k;
  for (k = 1; k <= 9; ++k) {

    // For each board, we call the evaluate_terminal function and store its returned value.
    int is_terminal_node = evaluate_terminal(k, current_player);

    // If we did find a terminal node, we return this value and stop searching this child node.
    if ((is_terminal_node == -100) | (is_terminal_node == 0)) {
      return is_terminal_node;
    }
  }

  // If the depth of the search equals 0, we don't want to search any deeper, and instead calculate
  // a heuristic value for this node. We use the function 3*X2 + X1 - (3*O2 + O1) for each board,
  // with the sum of all such values being our total heuristic value that we return.
  if (depth == 0) {
    int heuristic = 0;
    int j;
    for (j = 1; j <= 9; ++j) {

      // For all boards on the grid, we sum together their heuristic values which we calculate
      // using evaluate_heuristic.
      heuristic += evaluate_heuristic(j, current_player);
    }
    return heuristic;
  }

  // Now for each child of the current node, we can begin our alpha-beta search
  // using the negamax formulation.
  int i;
  for (i = 1; i <= 9; ++i) {

    // We first check if this position is already filled on the board.
    // If it is, playing here would be an illegal move, so we ignore this position and move on.
    if (board[current_board][i] == EMPTY) {

      // For the chosen position, we assign this move on the board.
      board[current_board][i] = current_player;

      // This is where we recursively call alpha_beta_search. We store the negative of the final
      // result in a variable, which we will later compare against our current alpha.
      // Like before, the new board is the same as the position we have chosen,
      // the depth is decreased by 1, alpha is -beta and beta is -alpha, and
      // we are playing from the perspective of the opponent, so player is !current_player.
      int search_result = -alpha_beta_search(i, depth - 1, -beta, -alpha, !current_player);

      // After attaining our results from the search, we can undo our move on this position.
      board[current_board][i] = EMPTY;

      // Here we are taking the max of our current alpha and the return value
      // of the alpha beta search, assigning this as alpha.
      if (search_result > alpha) {
        alpha = search_result;
      }

      // This is the pruning stage of the alpha-beta search, and is what allows the depth
      // to be much greater than what would be possible using regular minimax.
      // All we do is compare alpha and beta, and if alpha is greater or equal,
      // we can prune this section of the tree and return alpha.
      if (alpha >= beta) {
          return alpha;
      }
    }
  }

  // Finally we return alpha after searching all child nodes.
  return alpha;

}

/*********************************************************//*
   Evaluating if the current node is terminal
*/
int evaluate_terminal( int current_board, int current_player )
{

  // Firstly we want to evaluate if the current board has 3 in a row for the opponent,
  // making it a winning position.

  // We first check all the columns of the board. If an opponent has 3 in one column
  // we return -100 as the heuristic value.
  int i;
  for (i = 1; i <= 3; ++i) {
    if ((board[current_board][i] == !current_player)
    && (board[current_board][i + 3] == !current_player)
    && (board[current_board][i + 6] == !current_player)) {
      return -100;
    }
  }

  // We then check all the rows of the board. If an opponent has 3 in one row
  // we return -100 as the heuristic value.
  int j;
  for (j = 1; j <= 9; j = j + 3) {
    if ((board[current_board][j] == !current_player)
    && (board[current_board][j + 1] == !current_player)
    && (board[current_board][j + 2] == !current_player)) {
      return -100;
    }
  }

  // Finally we check both the diagonals of the board. If an opponent has 3 in one diagonal
  // we return -100 as the heuristic value.
  if ((board[current_board][1] == !current_player)
  && (board[current_board][5] == !current_player)
  && (board[current_board][9] == !current_player)) {
      return -100;
  } else if ((board[current_board][3] == !current_player)
  && (board[current_board][5] == !current_player)
  && (board[current_board][7] == !current_player)) {
      return -100;
  }

  // Our other check for a terminal node is a tie, meaning the board is filled.
  // This is tested by looking for an empty space, and if we find one we can exit
  // as it mustn't be full.
  int p;
  for (p = 1; p <= 9; ++p) {
    if (board[current_board][p] == EMPTY) {
      return -1;
    }
  }

  // If an empty position wasn't found on the board, it must be filled so we return 0
  // for a tie.
  return 0;

}

/*********************************************************//*
   If the depth of the alpha-beta search is 0, evaluate the heuristic value of this node
*/
int evaluate_heuristic( int current_board, int current_player )
{

  // When calculating the heuristic value of the node, we need to determine what
  // combination of pieces are in each row, column and diagonal of the board.
  // To make things easier, we will assume the current player is X, and the opponent
  // is O.

  // If there are 2 X's and no O's in a row, we increment the variable x2.
  // If there is 1 X and no O's, we increment the variable x1.
  // Similarly, we use the same logic for the variables o2 and o1.
  int x2 = 0;
  int x1 = 0;
  int o2 = 0;
  int o1 = 0;

  // We first check all the columns of the board. To make the various if statements
  // easier to follow, a comment is placed beside them to indicate what this
  // column would look like on the board.
  int i;
  for (i = 1; i <= 3; ++i) {
    if (board[current_board][i] == current_player) { // X
      if (board[current_board][i + 3] == current_player) { // XX
        if (board[current_board][i + 6] == EMPTY) { // XX_
          x2++;
        }
    } else if (board[current_board][i + 3] == EMPTY) { // X_
        if (board[current_board][i + 6] == current_player) { // X_X
            x2++;
        } else if (board[current_board][i + 6] == EMPTY) { // X__
            x1++;
        }
      }
    } else if (board[current_board][i] == !current_player) { // O
      if (board[current_board][i + 3] == !current_player) { // OO
        if (board[current_board][i + 6] == EMPTY) { // OO_
          o2++;
        }
      } else if (board[current_board][i + 3] == EMPTY) { // O_
        if (board[current_board][i + 6] == !current_player) { // O_O
          o2++;
        } else if (board[current_board][i + 6] == EMPTY) { // O__
          o1++;
        }
      }
    } else { // _
      if (board[current_board][i + 3] == current_player) { // _X
        if (board[current_board][i + 6] == current_player) { // _XX
          x2++;
        } else if (board[current_board][i + 6] == EMPTY) { // _X_
          x1++;
        }
      } else if (board[current_board][i + 3] == !current_player) { // _O
        if (board[current_board][i + 6] == !current_player) { // _OO
          o2++;
        } else if (board[current_board][i + 6] == EMPTY) { // _O_
          o1++;
        }
      } else { // __
        if (board[current_board][i + 6] == current_player) { // __X
          x1++;
        } else if (board[current_board][i + 6] == !current_player) { // __O
          o1++;
        }
      }
    }
  }

  // We then check all the rows of the board. To make the various if statements
  // easier to follow, a comment is placed beside them to indicate what this
  // row would look like on the board.
  int j;
  for (j = 1; j <= 9; j = j + 3) {
      if (board[current_board][j] == current_player) { // X
        if (board[current_board][j + 1] == current_player) { // XX
          if (board[current_board][j + 2] == EMPTY) { // XX_
            x2++;
          }
        } else if (board[current_board][j + 1] == EMPTY) { // X_
          if (board[current_board][j + 2] == current_player) { // X_X
              x2++;
          } else if (board[current_board][j + 2] == EMPTY) { // X__
              x1++;
          }
        }
      } else if (board[current_board][j] == !current_player) { // O
        if (board[current_board][j + 1] == !current_player) { // OO
          if (board[current_board][j + 2] == EMPTY) { // OO_
            o2++;
          }
        } else if (board[current_board][j + 1] == EMPTY) { // O_
          if (board[current_board][j + 2] == !current_player) { // O_O
            o2++;
          } else if (board[current_board][j + 2] == EMPTY) { // O__
            o1++;
          }
        }
      } else { // _
        if (board[current_board][j + 1] == current_player) { // _X
          if (board[current_board][j + 2] == current_player) { // _XX
            x2++;
          } else if (board[current_board][j + 2] == EMPTY) { // _X_
            x1++;
          }
        } else if (board[current_board][j + 1] == !current_player) { // _O
          if (board[current_board][j + 2] == !current_player) { // _OO
            o2++;
          } else if (board[current_board][j + 2] == EMPTY) { // _O_
            o1++;
          }
        } else { // __
          if (board[current_board][j + 2] == current_player) { // __X
            x1++;
          } else if (board[current_board][j + 2] == !current_player) { // __O
            o1++;
          }
        }
      }
  }

  // Finally, we check both the diagonals of the board. To make the various if statements
  // easier to follow, a comment is placed beside them to indicate what this
  // diagonal would look like on the board.
  if (board[current_board][1] == current_player) { // X
    if (board[current_board][5] == current_player) { // XX
        if (board[current_board][9] == EMPTY) { // XX_
            x2++;
        }
    } else if (board[current_board][5] == EMPTY) { // X_
        if (board[current_board][9] == current_player) { // X_X
            x2++;
        } else if (board[current_board][9] == EMPTY) { // X__
            x1++;
        }
    }
  } else if (board[current_board][1] == !current_player) { // O
    if (board[current_board][5] == !current_player) { // OO
        if (board[current_board][9] == EMPTY) { // OO_
            o2++;
        }
    } else if (board[current_board][5] == EMPTY) { // O_
        if (board[current_board][9] == !current_player) { // O_O
            o2++;
        } else if (board[current_board][9] == EMPTY) { // O__
            o1++;
        }
    }
  } else { // _
    if (board[current_board][5] == current_player) { // _X
      if (board[current_board][9] == current_player) { // _XX
        x2++;
      } else if (board[current_board][9] == EMPTY) { // _X_
        x1++;
      }
    } else if (board[current_board][5] == !current_player) { // _O
      if (board[current_board][9] == !current_player) { // _OO
        o2++;
      } else if (board[current_board][9] == EMPTY) { // _O_
        o1++;
      }
    } else { // __
      if (board[current_board][9] == current_player) { // __X
        x1++;
      } else if (board[current_board][9] == !current_player) { // __O
        o1++;
      }
    }
  }

  // This is the second diagonal to be calculated.
  if (board[current_board][3] == current_player) { // X
    if (board[current_board][5] == current_player) { // XX
        if (board[current_board][7] == EMPTY) { // XX_
            x2++;
        }
    } else if (board[current_board][5] == EMPTY) { // X_
        if (board[current_board][7] == current_player) { // X_X
            x2++;
        } else if (board[current_board][7] == EMPTY) { // X__
            x1++;
        }
    }
  } else if (board[current_board][3] == !current_player) { // O
    if (board[current_board][5] == !current_player) { // OO
        if (board[current_board][7] == EMPTY) { // OO_
            o2++;
        }
    } else if (board[current_board][5] == EMPTY) { // O_
        if (board[current_board][7] == !current_player) { // O_O
            o2++;
        } else if (board[current_board][7] == EMPTY) { // O__
            o1++;
        }
    }
  } else { // _
    if (board[current_board][5] == current_player) { // _X
      if (board[current_board][7] == current_player) { // _XX
        x2++;
      } else if (board[current_board][7] == EMPTY) { // _X_
        x1++;
      }
    } else if (board[current_board][5] == !current_player) { // _O
      if (board[current_board][7] == !current_player) { // _OO
        o2++;
      } else if (board[current_board][7] == EMPTY) { // _O_
        o1++;
      }
    } else { // __
      if (board[current_board][7] == current_player) { // __X
        x1++;
      } else if (board[current_board][7] == !current_player) { // __O
        o1++;
      }
    }
  }

  // After all values of x2, x1, o2 and o1 have been calculated for the current board,
  // we simply insert them into the heuristic function 3*X2 + X1 - (3*O2 + O1).
  // This value is then returned and added to the sum of all other heuristic values.
  int heuristic_function = 3*x2 + x1 - (3*o2 + o1);

  return heuristic_function;
}

/*********************************************************//*
   Receive last move and mark it on the board
*/
void agent_last_move( int prev_move )
{
  m++;
  move[m] = prev_move;
  board[move[m-1]][move[m]] = !player;
}

/*********************************************************//*
   Called after each game
*/
void agent_gameover(
                    int result,// WIN, LOSS or DRAW
                    int cause  // TRIPLE, ILLEGAL_MOVE, TIMEOUT or FULL_BOARD
                   )
{
  // nothing to do here
}

/*********************************************************//*
   Called after the series of games
*/
void agent_cleanup()
{
  // nothing to do here
}
