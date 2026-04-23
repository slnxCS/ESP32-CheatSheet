#include "input.hpp"

Button LeftButton(LEFT_BUTTON);
Button CenterButton(CENTER_BUTTON);
Button RightButton(RIGHT_BUTTON);

Input GetInput(void) 
{
    return Input(LeftButton.Update(), RightButton.Update(), CenterButton.Update());
}

Input GetFastInput(void)
{
    return Input(LeftButton.FastUpdate(), RightButton.FastUpdate(), CenterButton.FastUpdate());
}