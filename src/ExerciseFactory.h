#pragma once
#include <string>

class ExerciseFactory {
public:
    enum Mode {
        MODE_LETTERS = 0,
        MODE_SYLLABLES = 1,
        MODE_WORDS = 2,
        MODE_TEXT = 3
    };

    enum Row {
        ROW_ALL = 0,
        ROW_TOP = 1,
        ROW_MID = 2,
        ROW_BOT = 3
    };

    enum Difficulty {
        DIFF_EASY   = 0,
        DIFF_NORMAL = 1,
        DIFF_HARD   = 2
    };

    static std::wstring createExercise(Mode mode,
                                       int row = ROW_ALL,
                                       int durationSec = 60);

    static Difficulty difficultyFromDuration(int durationSec);
};