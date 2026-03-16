/* if(0) is never true, so the body should never execute.
   score should stay as 14, not get overwritten with 999. */
int f()
{
    int score;
    int bonus;
    score = 14;
    bonus = 3;
    if (0) {
        score = 999;
        bonus = 0;
    }
    return score;
}
