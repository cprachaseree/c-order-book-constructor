#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 50
#define LEVEL 10
#define BUY 1
#define SELL -1
#define MIN_PRICE 2900
#define MAX_PRICE 3200

#define NEXT_STR_INIT(x) strtok(x, ",")
#define NEXT_STR() strtok(NULL, ",")
#define NEXT_INT() atoi(strtok(NULL, ","))

static unsigned int price_points[MAX_PRICE - MIN_PRICE]; /* Entire limit orderbook by array of pricepoints */
static unsigned int ask_min; /* Minimum ask price, which is first level of ask orders */
static unsigned int bid_max; /* Maximum bid price, which is first level of bid orders */
static unsigned int ask_levels; /* ask price levels counts, used when executing and printing*/
static unsigned int bid_levels; /* bid price levels counts, used when executing and printing*/

static unsigned int buy_volume; 
static unsigned int buy_turnover;
static unsigned int sell_volume;
static unsigned int sell_turnover;

void init_static_variables(void)
{
    memset(price_points, 0, sizeof(int) * (MAX_PRICE - MIN_PRICE));
    ask_min = MAX_PRICE - MIN_PRICE + 1;
    bid_max = 0;
    ask_levels = 0;
    bid_levels = 0;
    buy_volume = 0;
    buy_turnover = 0;
    sell_volume = 0;
    sell_turnover = 0;
}

void submit_order(
    unsigned int qty,
    unsigned int price,
    unsigned int direction
) {
    if (price_points[price] == 0) { /* insert to new price level */
        if (direction == BUY) {
            if (price > bid_max) {
                bid_max = price;
            }
            bid_levels++;
        } else if (direction == SELL) {
            if (price < ask_min) {
                ask_min = price;
            }
            ask_levels++;
        }
    }
    price_points[price] += qty;
    printf("QTY: %d PRICE: %d\n", qty, price);
    printf("price_points[price]: %d\n", price_points[price]);
    printf("Changed price_points[%d] to be %d\n", price, price_points[price]);
    printf("BID MAX %d, ASK MIN %d\n", bid_max + MIN_PRICE, ask_min + MIN_PRICE);
}

void reduce_order(
    unsigned int qty,
    unsigned int price,
    unsigned int direction
) {
    if (qty >= price_points[price]) {
        price_points[price] = 0;
        return;
    }
    price_points[price] -= qty;
    if (price_points[price] == 0) { /* price level to remove */
        if (direction == BUY) {
            if (price == bid_max) { /* bid max is removed, so need to search for next bid max */
                while (price_points[bid_max] != 0) {
                    bid_max--;
                }
            }
            bid_levels--;
        } else if (direction == SELL) {
            if (price == ask_min) { /* ask min is removed, so need to search for next ask min */
                while (price_points[ask_min] != 0) {
                    ask_min++;
                }
            }
            ask_levels--;
        }
    }
}

int execute(
    unsigned int qty,
    unsigned int price,
    unsigned int next_level_direction,
    unsigned int *cur_price_level,
    unsigned int *volume,
    unsigned int *turnover
) {
    unsigned int deleted = 0;
    while (qty > 0 || *cur_price_level * next_level_direction <= price * next_level_direction) {
        if (price_points[*cur_price_level] != 0) { /* */
            *volume += qty;
            *turnover += qty * (*cur_price_level);
            if (price_points[*cur_price_level] >= qty) { /* */
                price_points[*cur_price_level] -= qty;
                qty = 0;
            } else { /* */
                qty -= price_points[*cur_price_level];
                price_points[*cur_price_level] = 0;
                *cur_price_level += next_level_direction;
                deleted++;
            }
        }
        *cur_price_level += next_level_direction;
    }
    if 
    price_points[*cur_price_level] += qty; /* Become a limit order */
    return deleted;
}

void execute_order(
    unsigned int qty,
    unsigned int price,
    unsigned int direction
) {
    unsigned int cur_price_level;
    unsigned int deleted;
    unsigned int volume;
    unsigned int turnover;

    if (direction == BUY) {
        printf("Executing buy\n");
        deleted = execute(qty, price, -1 * direction, &ask_min, &volume, &turnover);
        ask_levels -= deleted;
        buy_volume += volume;
        buy_turnover += turnover;
    } else if (direction == SELL) {
        printf("Executing sell\n");
        deleted = execute(qty, price, -1 * direction, &bid_max, &volume, &turnover);
        bid_levels -= deleted;
        sell_volume += volume;
        sell_turnover += turnover;
    }
}

void update_price_points(
    unsigned int type,
    unsigned int qty,
    unsigned int price,
    unsigned int direction
) {
    if (type == 1) {
        printf("Submitting order\n");
        submit_order(qty, price, direction);
    } else if (type == 2 || type == 3) {
        printf("Reducing order\n");
        reduce_order(qty, price, direction);
    } else if (type == 4 || type == 5) {
        printf("Executing order\n");
        execute_order(qty, price, direction);
    } else {
        printf("Invalid type %d\n", type);
        exit(EXIT_FAILURE);
    }
}

void print_book(void)
{
    printf("Print book.\n");
    unsigned int cur_price_level;
    unsigned int seen_levels;
    int remaining_levels;

    printf("BID: ");
    printf("Traded volume: %d, Traded notional: %d\n", buy_volume, buy_turnover);
    for (cur_price_level = bid_max, seen_levels = 0; cur_price_level > MIN_PRICE, seen_levels < bid_levels; cur_price_level--) {
        if (price_points[cur_price_level] != 0) {
            printf("%d@%d ", price_points[cur_price_level], cur_price_level + MIN_PRICE);
            seen_levels++;
        }
    }
    for (remaining_levels = LEVEL - seen_levels; remaining_levels > 0; remaining_levels--) {
        printf("0000@0000 ");
    }
    printf("\n");
    printf("ASK: ");
    printf("Traded volume: %d, Traded notional: %d\n", sell_volume, sell_turnover);
    for (cur_price_level = ask_min, seen_levels = 0; cur_price_level < MAX_PRICE, seen_levels < ask_levels; cur_price_level++) {
        if (price_points[cur_price_level] != 0) {
            printf("%d@%d ", price_points[cur_price_level], cur_price_level + MIN_PRICE);
            seen_levels++;
        }
    }
    for (remaining_levels = LEVEL - seen_levels; remaining_levels > 0; remaining_levels--) {
        printf("0000@0000 ");
    }
    printf("\n");
}

int main(int argv, char *argc[])
{
    FILE *fp;
    char line[BUF_SIZE];
    int loop = 0;
    int max_loop = 20;
    char *time;
    unsigned int type;
    char *order_id;
    unsigned int qty;
    unsigned int price;
    unsigned int direction;

    fp = fopen("LOBSTER_SampleFile_MSFT_2012-06-21_10/MSFT_2012-06-21_34200000_57600000_message_10.csv", "r");
    if (fp == NULL) {
        printf("File error\n");
        exit(EXIT_FAILURE);
    }

    init_static_variables();
    print_book();

    while (fgets(line, BUF_SIZE, fp) != NULL) {
        time = NEXT_STR_INIT(line);
        type = NEXT_INT();
        order_id = NEXT_STR();
        qty = NEXT_INT();
        price = NEXT_INT() / 100 - MIN_PRICE;
        direction = NEXT_INT();

        printf("LOOP %d READ LINE %d %d %d %d\n", loop, type, qty, price, direction);
        printf("Original price %d\n", price + MIN_PRICE);

        update_price_points(type, qty, price, direction);
        print_book();

        //if (++loop == max_loop) {
        //    break;
        //}
    }
	
    fclose(fp);
    return EXIT_SUCCESS;
}