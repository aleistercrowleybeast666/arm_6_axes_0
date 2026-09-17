#include "factory_recipes.h"
/* Independent per-product policies. No diameter, force target or process has been commissioned. */
static const FactoryRecipe recipes[3] = {{.track_index = 0,
                                          .force_policy = FORCE_POLICY_NONE,
                                          .end_action = FACTORY_END_HOLD,
                                          .steps = {{.type = RECIPE_STEP_PLAY_TRACK},
                                                    {.type = RECIPE_STEP_FORCE_HOOK},
                                                    {.type = RECIPE_STEP_DONE}}},
                                         {.track_index = 1,
                                          .force_policy = FORCE_POLICY_NONE,
                                          .end_action = FACTORY_END_HOLD,
                                          .steps = {{.type = RECIPE_STEP_PLAY_TRACK},
                                                    {.type = RECIPE_STEP_FORCE_HOOK},
                                                    {.type = RECIPE_STEP_DONE}}},
                                         {.track_index = 2,
                                          .force_policy = FORCE_POLICY_NONE,
                                          .end_action = FACTORY_END_HOLD,
                                          .steps = {{.type = RECIPE_STEP_PLAY_TRACK},
                                                    {.type = RECIPE_STEP_FORCE_HOOK},
                                                    {.type = RECIPE_STEP_DONE}}}};
const FactoryRecipe *FactoryRecipe_Get(unsigned i)
{
    return i < 3 ? &recipes[i] : NULL;
}
ArmResult FactoryRecipe_ForceHook(const FactoryRecipe *r, ForceConditionState *s, const ForceSnapshot *f,
                                  uint32_t now)
{
    if (!r)
        return ARM_INVALID_ARGUMENT;
    if (r->force_policy == FORCE_POLICY_NONE)
        return ARM_FINISHED;
    if (r->force_policy == FORCE_POLICY_RESERVED || r->force_policy == FORCE_POLICY_RIPPLE_BELOW)
        return ARM_NOT_CONFIGURED;
    return ForceCondition_Evaluate(&r->force_condition, s, f, now);
}
