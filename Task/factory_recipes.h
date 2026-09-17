#ifndef __FACTORY_RECIPES_H
#define __FACTORY_RECIPES_H
#include "force_sensor.h"
typedef enum
{
    RECIPE_STEP_PLAY_TRACK,
    RECIPE_STEP_MOVE_HOME,
    RECIPE_STEP_WAIT_FORCE_CONDITION,
    RECIPE_STEP_FORCE_HOOK,
    RECIPE_STEP_DONE,
    RECIPE_STEP_AUX
} RecipeStepType;
typedef enum
{
    FORCE_POLICY_NONE,
    FORCE_POLICY_CONTACT,
    FORCE_POLICY_RELEASE,
    FORCE_POLICY_STABLE,
    FORCE_POLICY_RIPPLE_BELOW,
    FORCE_POLICY_RESERVED
} FactoryForcePolicy;
typedef enum
{
    FACTORY_END_HOLD,
    FACTORY_END_MOVE_HOME,
    FACTORY_END_RESERVED
} FactoryEndAction;
typedef struct
{
    RecipeStepType type;
    ForceCondition condition;
} RecipeStep;
typedef struct
{
    unsigned track_index;
    FactoryForcePolicy force_policy;
    FactoryEndAction end_action;
    bool force_required;
    ForceCondition force_condition;
    RecipeStep steps[4];
} FactoryRecipe;
const FactoryRecipe *FactoryRecipe_Get(unsigned index);
ArmResult FactoryRecipe_ForceHook(const FactoryRecipe *recipe, ForceConditionState *state,
                                  const ForceSnapshot *force, uint32_t now_ms);
#endif
