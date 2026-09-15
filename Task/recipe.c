#include "recipe.h"
#include <math.h>
void Recipe_Init(RecipeBook *b)
{
    if (b != NULL)
        *b = (RecipeBook){.selected = RECIPE_CUSTOM};
}
ArmResult Recipe_Configure(RecipeBook *b, RecipeId id, const RecipeEntry *e)
{
    if (b == NULL || e == NULL || (unsigned)id >= RECIPE_COUNT || !e->configured ||
        !isfinite(e->geometry.diameter_mm) || !isfinite(e->geometry.height_mm) ||
        e->geometry.diameter_mm <= 0 || e->geometry.height_mm <= 0)
        return ARM_INVALID_ARGUMENT;
    b->entries[id] = *e;
    return ARM_OK;
}
ArmResult Recipe_Select(RecipeBook *b, RecipeId id)
{
    if (b == NULL || (unsigned)id >= RECIPE_COUNT)
        return ARM_INVALID_ARGUMENT;
    if (!b->entries[id].configured)
        return ARM_NOT_CONFIGURED;
    b->selected = id;
    return ARM_OK;
}
ArmResult Recipe_GetGeometry(const RecipeBook *b, CakeGeometry *g)
{
    if (b == NULL || g == NULL || (unsigned)b->selected >= RECIPE_COUNT)
        return ARM_INVALID_ARGUMENT;
    if (!b->entries[b->selected].configured)
        return ARM_NOT_CONFIGURED;
    *g = b->entries[b->selected].geometry;
    return ARM_OK;
}
ArmResult Recipe_GetSlots(const RecipeBook *b, uint16_t *teach, uint16_t *trajectory)
{
    CakeGeometry g;
    if (teach == NULL || trajectory == NULL)
        return ARM_INVALID_ARGUMENT;
    ArmResult r = Recipe_GetGeometry(b, &g);
    if (r != ARM_OK)
        return r;
    *teach = b->entries[b->selected].teach_slot;
    *trajectory = b->entries[b->selected].trajectory_slot;
    return ARM_OK;
}
