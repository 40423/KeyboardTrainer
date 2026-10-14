#include "ExerciseFactory.h"
#include "Exercise.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>
#include <cwctype>
#include <string>

// ============================================================
// ГПСЧ
// ============================================================
static std::mt19937& getRng()
{
    static std::mt19937 rng((unsigned)std::time(nullptr));
    return rng;
}

// ============================================================
// РЯДЫ КЛАВИАТУРЫ (по ТЗ)
// ============================================================
static const wchar_t* g_rowTop = L"йцукенгшщзхъ";
static const wchar_t* g_rowMid = L"фывапролджэ";
static const wchar_t* g_rowBot = L"ячсмитьбю";

// Возвращает указатель на строку ряда (без завершающего '\0').
// row == ROW_ALL => nullptr (означает «любая буква»).
static const wchar_t* rowChars(int row)
{
    switch (row) {
    case ExerciseFactory::ROW_TOP: return g_rowTop;
    case ExerciseFactory::ROW_MID: return g_rowMid;
    case ExerciseFactory::ROW_BOT: return g_rowBot;
    default:                       return nullptr; // ROW_ALL
    }
}

// true, если буква c входит в выбранный ряд.
// row == ROW_ALL => true для любой буквы.
static bool letterInRow(wchar_t c, int row)
{
    const wchar_t* r = rowChars(row);
    if (r == nullptr) return true;

    wchar_t low = normalizeChar(c);
    for (const wchar_t* p = r; *p; ++p) {
        if (normalizeChar(*p) == low) return true;
    }
    return false;
}

// true, если все буквы слова принадлежат выбранному ряду.
static bool wordInRow(const std::wstring& w, int row)
{
    if (row == ExerciseFactory::ROW_ALL) return true;
    for (wchar_t c : w) {
        if (!letterInRow(c, row)) return false;
    }
    return true;
}

// ============================================================
// БАЗА ТЕКСТОВ
// ============================================================
static const wchar_t* g_book_texts[] = {
    L"Мороз и солнце, день чудесный. Ещё ты дремлешь, друг прелестный? Пора, красавица, проснись.",
    L"У лукоморья дуб зелёный, златая цепь на дубе том. И днём и ночью кот учёный всё ходит по цепи кругом.",
    L"Мой дядя самых честных правил, когда не в шутку занемог.",
    L"Я вас любил: любовь ещё, быть может, в душе моей угасла не совсем.",
    L"Я помню чудное мгновенье: передо мной явилась ты, как мимолётное виденье.",
    L"Подруга дней моих суровых, голубка дряхлая моя! Одна в глуши лесов сосновых давно, давно ждёшь ты меня.",
    L"Буря мглою небо кроет, вихри снежные крутя. То, как зверь, она завоет, то заплачет, как дитя.",
    L"Любви, надежды, тихой славы недолго нежил нас обман. Исчезли юные забавы, как сон, как утренний туман.",
    L"Духовной жаждою томим, в пустыне мрачной я влачился. И шестикрылый серафим на перепутье мне явился.",
    L"В пустыне чахлой и скупой, на почве, зноем раскаленной, Анчар, как грозный часовой, стоит один во всей вселенной.",
    L"Белеет парус одинокий в тумане моря голубом. Что ищет он в стране далёкой?",
    L"Люблю отчизну я, но странною любовью. Ум мой не победит её.",
    L"Погиб поэт! Невольник чести пал, оклеветанный молвой.",
    L"Скажи-ка, дядя, ведь недаром Москва, спалённая пожаром, Французу отдана.",
    L"Выхожу один я на дорогу. Сквозь туман кремнистый путь блестит.",
    L"Печально я смотрю на наше поколенье. Его грядущее или пусто, или темно.",
    L"И скучно, и грустно, и некому руку подать в минуту душевной невзгоды.",
    L"Белая берёза под моим окном принакрылась снегом, точно серебром.",
    L"Не жалею, не зову, не плачу, всё пройдёт, как с белых яблонь дым.",
    L"Отговорила роща золотая берёзовым, весёлым языком. И журавли, печально пролетая, уж не жалеют ни о чём.",
    L"Ты жива ещё, моя старушка? Я тоже жив и здравствуй, ты моя.",
    L"Шагане ты моя, Шагане! Потому что я с севера, или что ли, Я готов тебе рассказать поле.",
    L"Люблю грозу в начале мая, когда весенний первый гром, как бы резвясь и играя, грохочет в небе голубом.",
    L"Есть в осени первоначальной короткая, но дивная пора.",
    L"Молчи, скрывайся и таи И чувства, и мечты свои. Пускай в душевной глубине Встают и заходят они.",
    L"Умом Россию не понять, Аршином общим не измерить. У ней особенная стать.",
    L"Я пришёл к тебе с приветом, рассказать, что солнце встало, что оно горячим светом по листам затрепетало.",
    L"Шёпот, робкое дыханье, Трели соловья. Серебро и колыханье Сонного ручья.",
    L"Ночь, улица, фонарь, аптека, бессмысленный и тусклый свет. Живи ещё хоть четверть века, всё будет так, исход нет.",
    L"Девушка пела в церковном хоре о всех усталых в чужом краю.",
    L"О, весна без конца и без краю, Без конца и без краю мечта! Узнаю тебя, жизнь, принимаю, И приветствую звоном щита.",
    L"Опять, как в годы золотые, Три стёртых стали в ряд.",
    L"Однажды в студёную зимнюю пору я из лесу вышел; был сильный мороз.",
    L"В каком году родился, Не думал и не гадал. На столбовой дорожке Семь мужиков сошлись.",
    L"Славная осень! Здоровый, ядрёный Воздух усталые силы бодрит.",
    L"Чуден Днепр в тихую погоду, когда вольно и плавно несёт сквозь горы и долины воды свои безмолвно.",
    L"Последний день перед Рождеством прошёл. Зимняя ясная ночь наступила.",
    L"Все счастливые семьи похожи друг на друга, каждая несчастливая семья несчастлива по-своему.",
    L"Нет величия там, где нет простоты, добра и правды.",
    L"В человеке всё должно быть прекрасно: и лицо, и одежда, и душа, и мысли.",
    L"Всё это было бы смешно, если бы не было так грустно.",
    L"В одной из отдалённых улиц Москвы стоял большой дом, с белыми колоннами.",
    L"Утро было свежее. Солнце поднималось над лугом.",
    L"Лес, точно терем расписным, лиловый, золотой, багряный и бурый, весёлой, пёстрою стеной стоит над светлою поляною.",
    L"Послушайте! Ведь, если звёзды зажигают, значит, это кому-нибудь нужно?",
    L"Светить всегда, светить везде, до дней последних донца.",
    L"Сжала руки под тёмным вуалем. Сегодня мне что-то не спится.",
    L"Моим стихам, написанным так рано, что не знала я, что я поэт.",
    L"Уж сколько раз твердили миру, что лесть гнусна, вредна.",
    L"Когда в товарищах согласья нет, На лад их дело не пойдёт.",
    L"Я вольный ветер, я вечно вею, волную волны, ласкаю ивы.",
    L"Переправа, переправа! Берег левый, берег правый. Снег жёсткий, край обрыва, Ночь как есть, куда ни плыть.",
    L"Служить бы рад, служить тошно мучно. Счастливые часов не наблюдают.",
    L"Человек есть тайна. Её надо разгадать, и не ленись на это время терять.",
    L"Красота спасёт мир. Но не всякая красота спасает.",
    L"Синяя бездна раскрывалась. Звёзды горят ярко. Ветер тихо подувает с моря.",
    L"Вера шла по саду. Солнце заходило за горизонт, и небо покрывалось золотом.",
    L"Тихий Дон широко разлился. Степь до горизонта тянется вокруг.",
    L"Мещерские леса полны тишины. Тихо в полях, тихо в лесах. И ничего не слышно, кроме шелеста.",
    L"А ведь есть люди, которые сами не знают, чего им надо.",
    L"Жил-был пискарь-идеалист. Жил он в реке, в тихом месте.",
    L"Когда император Александр Павлович путешествовал по Европе, он увидел в Англии удивительную вещь.",
    L"Жестокие нравы, сударь, в нашем городе, жестокие.",
    L"Что делать? Жить велит жизнь. Значит, надо жить.",
    L"Не хочу учиться, а хочу жениться.",
    L"Поэзия есть добродетель. И сладость, и великолепье.",
    L"Мой дар беден, и голос мой не громок.",
};

// ============================================================
// УТИЛИТЫ
// ============================================================
static int randomIndex(int count)
{
    if (count <= 0) return 0;
    return (int)(getRng()() % count);
}

// Собирает буквы из книг, отфильтрованные по ряду.
static std::wstring collectAllLetters(int row)
{
    std::wstring letters;
    int count = sizeof(g_book_texts) / sizeof(g_book_texts[0]);

    for (int i = 0; i < count; i++) {
        for (const wchar_t* p = g_book_texts[i]; *p; p++) {
            wchar_t c = *p;
            if (c == L' ' || c == L',' || c == L'.' ||
                c == L'!' || c == L'?' || c == L':' || c == L';')
                continue;
            if (!letterInRow(c, row)) continue;
            letters += c;
        }
    }
    return letters;
}

// Собирает слова из книг, отфильтрованные по ряду.
static std::vector<std::wstring> collectAllWords(int row)
{
    std::vector<std::wstring> words;
    int count = sizeof(g_book_texts) / sizeof(g_book_texts[0]);

    for (int i = 0; i < count; i++) {
        std::wstring current;
        for (const wchar_t* p = g_book_texts[i]; *p; p++) {
            wchar_t c = *p;
            if (c == L' ' || c == L',' || c == L'.' ||
                c == L'!' || c == L'?' || c == L':' || c == L';') {
                if (!current.empty()) {
                    if (wordInRow(current, row)) words.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
        if (!current.empty() && wordInRow(current, row)) {
            words.push_back(current);
        }
    }
    return words;
}

// Случайная буква.
// Для ROW_ALL — из книг (кэш).
// Для конкретного ряда — напрямую из таблицы ряда,
// чтобы были доступны все буквы ряда, даже редкие.
static wchar_t randomLetterFromBooks(int row)
{
    if (row == ExerciseFactory::ROW_ALL) {
        static std::wstring letters = collectAllLetters(ExerciseFactory::ROW_ALL);
        if (letters.empty()) return L'а';
        return letters[randomIndex((int)letters.size())];
    }

    const wchar_t* r = rowChars(row);
    if (r == nullptr || r[0] == L'\0') return L'а';
    int len = (int)wcslen(r);
    return r[randomIndex(len)];
}

// Случайный слог (2 буквы подряд из слова), отфильтрованный по ряду.
static std::wstring randomSyllableFromWords(int row)
{
    // Кэш по ряду: 0=ALL, 1=TOP, 2=MID, 3=BOT
    static std::vector<std::wstring> cache[4];
    static bool initialized[4] = { false, false, false, false };

    int slot = (row >= 0 && row <= 3) ? row : 0;
    if (!initialized[slot]) {
        cache[slot] = collectAllWords(row);
        initialized[slot] = true;
    }

    const std::vector<std::wstring>& words = cache[slot];
    if (words.empty()) {
        std::wstring s;
        s += randomLetterFromBooks(row);
        s += randomLetterFromBooks(row);
        return s;
    }

    for (int attempt = 0; attempt < 50; attempt++) {
        const std::wstring& w = words[randomIndex((int)words.size())];
        if (w.length() < 2) continue;

        int pos = randomIndex((int)w.length() - 1);
        std::wstring syll;
        syll += w[pos];
        syll += w[pos + 1];

        if (!iswalpha(syll[0]) || !iswalpha(syll[1])) continue;

        bool hasVowel = false;
        for (wchar_t c : syll) {
            wchar_t low = towlower(c);
            if (low == L'а' || low == L'е' || low == L'ё' ||
                low == L'и' || low == L'о' || low == L'у' ||
                low == L'ы' || low == L'э' || low == L'ю' ||
                low == L'я')
            {
                hasVowel = true;
                break;
            }
        }
        if (hasVowel) return syll;
    }

    std::wstring s;
    s += randomLetterFromBooks(row);
    s += randomLetterFromBooks(row);
    return s;
}

// N случайных букв через пробел.
static std::wstring lettersThroughSpace(int count, int row)
{
    std::wstring result;
    for (int i = 0; i < count; i++) {
        if (i > 0) result += L' ';
        result += randomLetterFromBooks(row);
    }
    return result;
}

// N случайных слогов через пробел.
static std::wstring syllablesThroughSpace(int count, int row)
{
    std::wstring result;
    for (int i = 0; i < count; i++) {
        if (i > 0) result += L' ';
        result += randomSyllableFromWords(row);
    }
    return result;
}

// Случайные слова, пока не наберётся charTarget символов.
static std::wstring randomWordsJoined(int charTarget, int row)
{
    static std::vector<std::wstring> cache[4];
    static bool initialized[4] = { false, false, false, false };

    int slot = (row >= 0 && row <= 3) ? row : 0;
    if (!initialized[slot]) {
        cache[slot] = collectAllWords(row);
        initialized[slot] = true;
    }

    const std::vector<std::wstring>& words = cache[slot];
    if (words.empty()) return L"текст";

    std::wstring result;
    int safetyCounter = 0;
    while ((int)result.length() < charTarget && safetyCounter < 1000) {
        if (!result.empty()) result += L' ';
        result += words[randomIndex((int)words.size())];
        safetyCounter++;
    }
    return result;
}

// Случайные тексты (только для ROW_ALL — по ТЗ).
static std::wstring randomTextJoined(int charTarget)
{
    std::wstring result;
    int count = sizeof(g_book_texts) / sizeof(g_book_texts[0]);
    if (count == 0) return L"текст";

    int safetyCounter = 0;
    while ((int)result.length() < charTarget && safetyCounter < 100) {
        if (!result.empty()) result += L' ';
        result += g_book_texts[randomIndex(count)];
        safetyCounter++;
    }
    return result;
}

// ============================================================
// ГЛАВНАЯ ФУНКЦИЯ
// ============================================================
ExerciseFactory::Difficulty ExerciseFactory::difficultyFromDuration(int d)
{
    if (d <= 30) return DIFF_EASY;
    if (d <= 60) return DIFF_NORMAL;
    return DIFF_HARD;
}

std::wstring ExerciseFactory::createExercise(Mode mode, int row, int durationSec)
{
    // Страховка от некорректного row.
    if (row < ROW_ALL || row > ROW_BOT) row = ROW_ALL;

    // По ТЗ «Слова» и «Текст» доступны только для ряда «Все».
    if ((mode == MODE_WORDS || mode == MODE_TEXT) && row != ROW_ALL) {
        row = ROW_ALL;
    }

    int targetChars = durationSec * 4;
    if (targetChars < 20) targetChars = 20; // защита от durationSec==0

    switch (mode) {
    case MODE_LETTERS:
    {
        int count = targetChars / 2;
        if (count < 10) count = 10;
        return lettersThroughSpace(count, row);
    }

    case MODE_SYLLABLES:
    {
        int count = targetChars / 3;
        if (count < 8) count = 8;
        return syllablesThroughSpace(count, row);
    }

    case MODE_WORDS:
    {
        return randomWordsJoined(targetChars, row);
    }

    case MODE_TEXT:
    default:
    {
        return randomTextJoined(targetChars);
    }
    }
}