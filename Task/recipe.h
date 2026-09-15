#ifndef __RECIPE_H
#define __RECIPE_H
#include "robot_math_types.h"
typedef enum
{
    RECIPE_SMALL,
    RECIPE_MEDIUM,
    RECIPE_LARGE,
    RECIPE_CUSTOM,
    RECIPE_COUNT
} RecipeId;
typedef struct
{
    float diameter_mm, height_mm;
} CakeGeometry;
typedef struct
{
    CakeGeometry geometry;
    uint16_t teach_slot, trajectory_slot;
    bool configured;
} RecipeEntry;
typedef struct
{
    RecipeEntry entries[RECIPE_COUNT];
    RecipeId selected;
} RecipeBook;
void Recipe_Init(RecipeBook *book);
ArmResult Recipe_Configure(RecipeBook *book, RecipeId id, const RecipeEntry *entry);
ArmResult Recipe_Select(RecipeBook *book, RecipeId id);
ArmResult Recipe_GetGeometry(const RecipeBook *book, CakeGeometry *geometry);
ArmResult Recipe_GetSlots(const RecipeBook *book, uint16_t *teach_slot, uint16_t *trajectory_slot);
#endif
