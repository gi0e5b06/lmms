/*
 */

#include "ScoreGDXView.h"

#include "GuiApplication.h"
#include "MainWindow.h"
#include "Note.h"
#include "Pattern.h"
#include "PianoRoll.h"
#include "embed.h"
#include "gui_templates.h"

#include <QPainter>

ScoreGDXView::ScoreGDXView(ToolPlugin* plugin) : ToolPluginView(plugin)
{
    setBaseSize(500, 250);
    setMinimumSize(500, 250);

    /*
    SubWindow* w
            = SubWindow::putWidgetOnWorkspace(this, true, false, true, false);
    w->adjustSize();
    w->update();
    */
    // hide();
    if(parentWidget())
    {
        parentWidget()->adjustSize();
        parentWidget()->update();
        parentWidget()->hide();
    }

    connect(gui->pianoRollWindow(), SIGNAL(currentPatternChanged()), this,
            SLOT(update()));
}

ScoreGDXView::~ScoreGDXView()
{
}

bool ScoreGDXView::drawStave(QPainter& p, int& x, int& y, int w, int h)
{
    p.setPen(Qt::gray);
    for(int i = 0; i <= 4; i++)
        p.drawLine(x, y - 6 * i - 1, w - 1, y - 6 * i - 1);
    return true;
}

bool ScoreGDXView::drawTempo(
        QPainter& p, int& x, int& y, int w, int h, bpm_t _tempo)
{
    p.setPen(Qt::black);
    p.setFont(QFont("DejaVu Sans", 14));
    p.drawText(x, y - 50, QString("𝅘𝅥"));
    p.setFont(QFont("DejaVu Sans", 8));
    p.drawText(x + 6, y - 50, QString(" = %1").arg(_tempo));
    return true;
}

bool ScoreGDXView::drawClef(
        QPainter& p, int& x, int& y, int w, int h, int _clef)
{
    static const QString clefs[] = {"𝄞", "𝄢", "𝄡", "𝄡", "𝄥", "𝄦", "𝄠", "𝄟",
                                    "𝄤", "𝄣", "𝄞", "𝄡", "𝄡", "𝄡", "𝄢", "𝄢"};
    static const QString names[]
            = {"Trebble",       "Bass",      "Alto",           "Tenor",
               "Perc",          "Perc",      "Down 8 Trebble", "Up 8 Trebble",
               "Down 8 bass",   "Up 8 bass", "French Violin",  "Soprano",
               "Mezzo-soprano", "Baritone",  "Baritone",       "Subbass"};

    static const int dy[]
            = {0, 0, -1, -7, 0, 0, 0, 0, 0, 0, +6, +11, +7, -13, -6, +6};

    // if(x + 24 > w)
    //    return false;

    p.setFont(QFont("DejaVu Sans", 24));
    p.setPen(Qt::black);
    p.drawText(x, y + dy[_clef], clefs[_clef]);
    x += 24;
    return true;
}

bool ScoreGDXView::drawBar(
        QPainter& p, int& x, int& y, int w, int h, int _bar)
{
    static const QString bars[]
            = {"𝄀", "𝄁", "𝄂", "𝄃", "𝄄", "𝄅", "𝄆", "𝄇", "𝄈"};

    // if(x + 24 > w)
    //    return false;

    p.setFont(QFont("DejaVu Sans", 24));
    p.setPen(Qt::black);
    p.drawText(x, y - 1, bars[_bar]);
    x += 12;
    return true;
}

bool ScoreGDXView::drawSignature(
        QPainter& p, int& x, int& y, int w, int h, int _n, int _d)
{
    static const QString signatures[] = {"𝄴", "𝄵"};

    // if(x + 24 > w)
    //    return false;

    if(_n == 4 && _d == 4)
    {
        p.setFont(QFont("DejaVu Sans", 24));
        p.setPen(Qt::black);
        p.drawText(x, y, signatures[0]);
    }
    else if(_n == 2 && _d == 2)
    {
        p.setFont(QFont("DejaVu Sans", 24));
        p.setPen(Qt::black);
        p.drawText(x, y, signatures[1]);
    }
    else
    {
        p.setFont(QFont("DejaVu Sans", 13, QFont::Bold));
        p.setPen(Qt::black);
        p.drawText(x, y, QString("%1").arg(_d));
        p.drawText(x, y - 13, QString("%1").arg(_n));
    }
    x += 24;
    return true;
}

bool ScoreGDXView::drawNote(
        QPainter& p, int& x, int& y, int w, int h, int _key, int _ticks)
{
    const bool blacks[]  = {false, true,  false, true,  false, false,
                           true,  false, true,  false, true,  false};
    const int  heights[] = {21, 21, 18, 18, 15, 12, 12, 9, 9, 6, 6, 3};

    const int     lengths[] = {720, 672, 576, -1,  384,  // double
                           360, 336, 288, 256, 192,  // whole
                           180, 168, 144, 128, 96,   // half
                           90,  84,  72,  64,  48,   // quarter
                           45,  42,  36,  32,  24,   // 1/8
                           -1,  21,  18,  16,  12,   // 1/16
                           -1,  -1,  9,   8,   6,    // 1/32
                           -1,  -1,  -1,  4,   3,    // 1/64
                           -1,  -1,  -1,  2,   -1,   // 1/128
                           -1,  -1,  -1,  -1,  1};   // 1/192
    const QString notes[]
            = {"𝅜𝅭𝅭𝅭", "𝅜𝅭𝅭", "𝅜𝅭", "",   "𝅜",   // double, breve
               "𝅝𝅭𝅭𝅭", "𝅝𝅭𝅭", "𝅝𝅭", "𝅜3", "𝅝",   // whole, semibreve
               "𝅗𝅥𝅭𝅭𝅭", "𝅗𝅥𝅭𝅭", "𝅗𝅥𝅭", "𝅝3", "𝅗𝅥",   // half, minim
               "𝅘𝅥𝅭𝅭𝅭", "𝅘𝅥𝅭𝅭", "𝅘𝅥𝅭", "𝅗𝅥3", "𝅘𝅥",   // quarter, crotchet
               "𝅘𝅥𝅮𝅭𝅭𝅭", "𝅘𝅥𝅮𝅭𝅭", "𝅘𝅥𝅮𝅭", "𝅘𝅥3", "𝅘𝅥𝅮",   // 1/8, quaver
               "𝅘𝅥𝅯𝅭𝅭𝅭", "𝅘𝅥𝅯𝅭𝅭", "𝅘𝅥𝅯𝅭", "𝅘𝅥𝅮3", "𝅘𝅥𝅯",   // 1/16, semiquaver
               "𝅘𝅥𝅰𝅭𝅭𝅭", "𝅘𝅥𝅰𝅭𝅭", "𝅘𝅥𝅰𝅭", "𝅘𝅥𝅯3", "𝅘𝅥𝅰",   // 1/32, demisemiquaver
               "𝅘𝅥𝅱𝅭𝅭𝅭", "𝅘𝅥𝅱𝅭𝅭", "𝅘𝅥𝅱𝅭", "𝅘𝅥𝅰3", "𝅘𝅥𝅱",   // 1/64
               "𝅘𝅥𝅲𝅭𝅭𝅭", "𝅘𝅥𝅲𝅭𝅭", "𝅘𝅥𝅲𝅭", "𝅘𝅥𝅱3", "𝅘𝅥𝅲",   // 1/128
               "",     "",    "",   "",   "𝅭"};  // 1/192
    // const QString dot = "𝅭";
    const QString alts[] = {"♭", "♮", "♯"};

    const int TPT    = MidiTime::ticksPerTact();
    int       istart = 0;
    while(lengths[istart] > TPT)
        istart++;

    // if(x + 24 > w)
    //    return false;

    int dy   = -12 + heights[_key % 12] - 21 * ((_key / 12) - 5);
    int ymin = 10000;
    int nn   = 0;
    int x0   = x;
    while(_ticks >= 1)
    {
        for(int i = istart; i < 50; i++)
            if(lengths[i] > 0 && _ticks >= lengths[i])
            {
                _ticks -= lengths[i];
                if(_key <= 64)
                {
                    p.setPen(Qt::gray);
                    for(int k = 64; k >= _key - 1; k--)
                    {
                        if(blacks[k % 12])
                            continue;
                        // if(heights[k % 12] % 6 == 0)
                        {
                            int dyl = -13 + (heights[k % 12] / 6) * 6
                                      - 21 * ((k / 12) - 5);
                            if(blacks[_key % 12])
                                x += 13;
                            p.drawLine(x - 3, y + dyl, x + 11, y + dyl);
                            if(blacks[_key % 12])
                                x -= 13;
                        }
                    }
                }
                if(_key >= 79)
                {
                    p.setPen(Qt::gray);
                    for(int k = 79; k <= _key + 2; k++)
                    {
                        if(blacks[k % 12])
                            continue;
                        // if(heights[k % 12] % 6 == 0)
                        {
                            int dyl = -13 + (heights[k % 12] / 6) * 6
                                      - 21 * ((k / 12) - 5);
                            if(blacks[_key % 12])
                                x += 13;
                            p.drawLine(x - 3, y + dyl, x + 11, y + dyl);
                            if(blacks[_key % 12])
                                x -= 13;
                        }
                    }
                }
                // p.setPen(Qt::white);
                // p.drawText(x, y + dy, notes[i]);
                p.setPen(Qt::black);
                if(blacks[_key % 12])
                {
                    p.setFont(QFont("DejaVu Sans", 24));
                    p.drawText(x, y + dy + 11, alts[2]);
                }
                x += 13;

                QString s = notes[i];
                if(s.right(1) == "3")
                {
                    s = s.left(s.length() - 1);
                    p.setFont(QFont("DejaVu Sans", 8));
                    p.drawText(x + 1, y + dy - 18, "3");
                }
                p.setFont(QFont("DejaVu Sans", 24));
                p.drawText(x, y + dy, s);
                x += 10 + 3 * s.length();
                nn++;
                ymin = qMin(ymin, y + dy);
                break;
            }
    }

    if(nn > 1)
    {
        p.setPen(Qt::black);
        ymin = (ymin - 32) / 6 * 6 + 1;
        p.drawLine(x0, ymin + 2, x0 + 2, ymin);
        p.drawLine(x0 + 2, ymin, x - 12, ymin);
        p.drawLine(x - 12, ymin, x - 10, ymin + 2);
    }

    x += 4;
    return true;
}

bool ScoreGDXView::drawChord(QPainter&    p,
                             int&         x,
                             int&         y,
                             int          w,
                             int          h,
                             QVector<int> _keys,
                             int          _ticks)
{
    const bool blacks[]  = {false, true,  false, true,  false, false,
                           true,  false, true,  false, true,  false};
    const int  heights[] = {21, 21, 18, 18, 15, 12, 12, 9, 9, 6, 6, 3};
    //                      C   C   D   D   E   F   F   G  G  A  A  B

    const int     lengths[] = {720, 672, 576, -1,  384,  // double
                           360, 336, 288, 256, 192,  // whole
                           180, 168, 144, 128, 96,   // half
                           90,  84,  72,  64,  48,   // quarter
                           45,  42,  36,  32,  24,   // 1/8
                           -1,  21,  18,  16,  12,   // 1/16
                           -1,  -1,  9,   8,   6,    // 1/32
                           -1,  -1,  -1,  4,   3,    // 1/64
                           -1,  -1,  -1,  2,   -1,   // 1/128
                           -1,  -1,  -1,  -1,  1};   // 1/192
    const QString notes[]
            = {"𝅜𝅭𝅭𝅭", "𝅜𝅭𝅭", "𝅜𝅭", "",   "𝅜",   // double, breve
               "𝅝𝅭𝅭𝅭", "𝅝𝅭𝅭", "𝅝𝅭", "𝅜3", "𝅝",   // whole, semibreve
               "𝅗𝅥𝅭𝅭𝅭", "𝅗𝅥𝅭𝅭", "𝅗𝅥𝅭", "𝅝3", "𝅗𝅥",   // half, minim
               "𝅘𝅥𝅭𝅭𝅭", "𝅘𝅥𝅭𝅭", "𝅘𝅥𝅭", "𝅗𝅥3", "𝅘𝅥",   // quarter, crotchet
               "𝅘𝅥𝅮𝅭𝅭𝅭", "𝅘𝅥𝅮𝅭𝅭", "𝅘𝅥𝅮𝅭", "𝅘𝅥3", "𝅘𝅥𝅮",   // 1/8, quaver
               "𝅘𝅥𝅯𝅭𝅭𝅭", "𝅘𝅥𝅯𝅭𝅭", "𝅘𝅥𝅯𝅭", "𝅘𝅥𝅮3", "𝅘𝅥𝅯",   // 1/16, semiquaver
               "𝅘𝅥𝅰𝅭𝅭𝅭", "𝅘𝅥𝅰𝅭𝅭", "𝅘𝅥𝅰𝅭", "𝅘𝅥𝅯3", "𝅘𝅥𝅰",   // 1/32, demisemiquaver
               "𝅘𝅥𝅱𝅭𝅭𝅭", "𝅘𝅥𝅱𝅭𝅭", "𝅘𝅥𝅱𝅭", "𝅘𝅥𝅰3", "𝅘𝅥𝅱",   // 1/64
               "𝅘𝅥𝅲𝅭𝅭𝅭", "𝅘𝅥𝅲𝅭𝅭", "𝅘𝅥𝅲𝅭", "𝅘𝅥𝅱3", "𝅘𝅥𝅲",   // 1/128
               "",     "",    "",   "",   "𝅭"};  // 1/192
    // const QString dot = "𝅭";
    const QString alts[] = {"♭", "♮", "♯"};

    const int TPT    = MidiTime::ticksPerTact();
    int       istart = 0;
    while(lengths[istart] > TPT)
        istart++;

    // if(x + 24 > w)
    //    return false;

    int ymin = 10000;
    int nn   = 0;
    int x0   = x;

    for(int n = 0; n < _keys.size(); n++)
    {
        x = x0;

        const int  key  = _keys[n];
        const bool last = (n == _keys.size() - 1);

        int dy    = -12 + heights[key % 12] - 21 * ((key / 12) - 5);
        int ticks = _ticks;

        while(ticks >= 1)
        {
            for(int i = istart; i < 50; i++)
                if(lengths[i] > 0 && ticks >= lengths[i])
                {
                    ticks -= lengths[i];

                    if(key <= 64)
                    {
                        p.setPen(Qt::gray);
                        for(int i = -1 + (key - 64) / 3; i <= -1; i++)
                            p.drawLine(x + 6, y - 6 * i - 1, x + 30,
                                       y - 6 * i - 1);
                    }
                    if(key >= 79)
                    {
                        p.setPen(Qt::gray);
                        for(int i = 5; i <= 5 + (key - 79) / 3; i++)
                            p.drawLine(x + 6, y - 6 * i - 1, x + 30,
                                       y - 6 * i - 1);
                    }

                    p.setPen(Qt::black);
                    if(blacks[key % 12])
                    {
                        p.setFont(QFont("DejaVu Sans", 24));
                        p.drawText(x, y + dy + 11, alts[2]);
                    }
                    x += 13;

                    QString s = notes[i];
                    if(s.right(1) == "3")
                    {
                        s = s.left(s.length() - 1);
                        if(last)
                        {
                            p.setFont(QFont("DejaVu Sans", 8));
                            p.drawText(x + 1, y + dy - 18, "3");
                        }
                    }
                    if(!last)
                    {
                        int j = i;
                        if(j % 5 == 3)
                            j++;
                        if(j >= 20)
                            j = 15 + j % 5;
                        if(i != j)
                            qInfo(" i %d -> j %d", i, j);
                        s = notes[j];
                    }
                    p.setFont(QFont("DejaVu Sans", 24));
                    p.drawText(x, y + dy, s);
                    x += 12 + 3 * s.length();
                    if(last)
                        nn++;
                    ymin = qMin(ymin, y + dy);
                    break;
                }
        }
    }

    if(nn > 1)
    {
        p.setPen(Qt::black);
        ymin = (ymin - 32) / 6 * 6 + 1;
        p.drawLine(x0 + 18, ymin + 2, x0 + 20, ymin);
        p.drawLine(x0 + 20, ymin, x - 13, ymin);
        p.drawLine(x - 13, ymin, x - 11, ymin + 2);
    }
    x += 4;

    return true;
}

bool ScoreGDXView::drawSilence(
        QPainter& p, int& x, int& y, int w, int h, int _ticks)
{
    const int     lengths[] = {720, 672, 576, -1,  384,          // double
                           360, 336, 288, 256, 192,          // whole
                           180, 168, 144, 128, 96,           // half
                           90,  84,  72,  64,  48,           // quarter
                           45,  42,  36,  32,  24,           // 1/8
                           -1,  21,  18,  16,  12,           // 1/16
                           -1,  -1,  9,   8,   6,            // 1/32
                           -1,  -1,  -1,  4,   3,            // 1/64
                           -1,  -1,  -1,  2,   -1,           // 1/128
                           -1,  -1,  -1,  -1,  1};           // 1/192
    const QString silences[] = {"𝄺𝅭𝅭𝅭", "𝄺𝅭𝅭", "𝄺𝅭", "𝄺3", "𝄺",  //
                                "𝄻𝅭𝅭𝅭", "𝄻𝅭𝅭", "𝄻𝅭", "𝄻3", "𝄻",  //
                                "𝄼𝅭𝅭𝅭", "𝄼𝅭𝅭", "𝄼𝅭", "𝄼3", "𝄼",  //
                                "𝄽𝅭𝅭𝅭", "𝄽𝅭𝅭", "𝄽𝅭", "𝄽3", "𝄽",  //
                                "𝄾𝅭𝅭𝅭", "𝄾𝅭𝅭", "𝄾𝅭", "𝄾3", "𝄾",  //
                                "𝄿𝅭𝅭𝅭", "𝄿𝅭𝅭", "𝄿𝅭", "𝄿3", "𝄿",  //
                                "𝅀𝅭𝅭𝅭", "𝅀𝅭𝅭", "𝅀𝅭", "𝅀3", "𝅀",  //
                                "𝅁𝅭𝅭𝅭", "𝅁𝅭𝅭", "𝅁𝅭", "𝅁3", "𝅁",  //
                                "𝅂𝅭𝅭𝅭", "𝅂𝅭𝅭", "𝅂𝅭", "𝅂3", "𝅂",  //
                                "",     "",    "",   "",   "𝅭"};
    // const QString dot = "𝅭";

    const int TPT    = MidiTime::ticksPerTact();
    int       istart = 0;
    while(lengths[istart] > TPT)
        istart++;

    // if(x + 24 > w)
    //    return false;

    int dy   = -1;
    int ymin = 10000;
    int nn   = 0;
    int x0   = x;
    while(_ticks >= 1)
    {
        for(int i = istart; i < 50; i++)
            if(lengths[i] > 0 && _ticks >= lengths[i])
            {
                _ticks -= lengths[i];
                p.setPen(Qt::black);
                QString s = silences[i];
                if(s.right(1) == "3")
                {
                    s = s.left(s.length() - 1);
                    p.setFont(QFont("DejaVu Sans", 8));
                    p.drawText(x + 1, y + dy - 18, "3");
                }
                p.setFont(QFont("DejaVu Sans", 24));
                p.drawText(x + 5, y + dy, s);
                x += 12 + 3 * s.length();
                nn++;
                ymin = qMin(ymin, y + dy);
                break;
            }
    }

    if(nn > 1)
    {
        p.setPen(Qt::black);
        ymin = (ymin - 32) / 6 * 6 + 1;
        p.drawLine(x0, ymin + 2, x0 + 2, ymin);
        p.drawLine(x0 + 2, ymin, x - 12, ymin);
        p.drawLine(x - 12, ymin, x - 10, ymin + 2);
    }
    x += 4;
    return true;
}

void ScoreGDXView::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    // p.fillRect( QRect( 0, 0, width(), height() ), Qt::red );
    p.fillRect(QRect(0, 0, width(), height()), Qt::white);
    p.setPen(Qt::black);
    // U+1D15D - U+1D164

    const int TPT = MidiTime::ticksPerTact();

    bool firstStave = true;
    bool firstNote  = true;
    int  w          = width() - 2;
    int  h          = 80;
    int  x          = 2;
    int  y          = 20 + h;

    const Pattern* tile = gui->pianoRollWindow()->currentPattern();
    if(tile != nullptr)
    {
        tick_t b      = TPT;
        tick_t start  = -1;
        tick_t end    = 0;
        int    xstart = 0;
        for(const Chord& chord: tile->chords())
        {
            if(chord.isEmpty())
                continue;
            const Note* note = chord[0];
            if(!firstStave && note->pos() >= b)
            {
                b += TPT;
                if(end % TPT != 0)
                {
                    drawSilence(p, x, y, w, h, TPT - end % TPT);
                    end = end - end % TPT + TPT;
                }
                if(w - x < 202)
                {
                    x = w - 1;
                    drawBar(p, x, y, w, h, 0);
                    x = 2;
                    y += h;
                    firstNote = true;
                }
                if(!firstNote)
                    drawBar(p, x, y, w, h, 0);
            }
            if(firstNote)
            {
                drawStave(p, x, y, w, h);
                drawBar(p, x, y, w, h, 0);
                if(firstStave)
                    drawTempo(p, x, y, w, h, Engine::song()->getTempo());
                drawClef(p, x, y, w, h, 0);
                if(firstStave)
                {
                    MeterModel& ts = Engine::song()->getTimeSigModel();
                    drawSignature(p, x, y, w, h, ts.numeratorModel().value(),
                                  ts.denominatorModel().value());
                }
                else
                    x += 8;
                firstStave = false;
            }

            if(note->pos().ticks() > end)
                drawSilence(p, x, y, w, h, note->pos().ticks() - end);

            if(note->pos().ticks() == start)
            {
                x = xstart;
            }
            else
            {
                start  = note->pos().ticks();
                xstart = x;
            }

            // drawNote(p, x, y, w, h, note->key(), note->length().ticks());
            QVector<int> vk;
            QString      deb;
            for(const Note* sn: chord)
            {
                vk.append(sn->key());
                deb += QString(" %1").arg(sn->key());
            }
            if(vk.size() > 1)
                qInfo("Chord:%s", qPrintable(deb));

            drawChord(p, x, y, w, h, vk, note->length().ticks());
            end = qMax(end, note->pos().ticks() + note->length().ticks());
            firstNote = false;
        }

        p.fillRect(QRect(x + 4, y - 25, w - x - 4, h), Qt::white);
        if(!firstNote)
            drawBar(p, x, y, w, h, 2);

        /*
        tick_t b      = TPT;
        tick_t start  = -1;
        tick_t end    = 0;
        int    xstart = 0;
        for(const Note* note: tile->notes())
        {
            if(!firstStave && note->pos() >= b)
            {
                b += TPT;
                if(w - x < 202)
                {
                    x = w - 1;
                    drawBar(p, x, y, w, h, 0);
                    x = 2;
                    y += h;
                    firstNote = true;
                }
                if(!firstNote)
                    drawBar(p, x, y, w, h, 0);
            }
            if(firstNote)
            {
                drawStave(p, x, y, w, h);
                drawBar(p, x, y, w, h, 0);
                if(firstStave)
                    drawTempo(p, x, y, w, h, 140);
                drawClef(p, x, y, w, h, 0);
                if(firstStave)
                    drawSignature(p, x, y, w, h, 3, 4);
                else
                    x += 8;
                firstStave = false;
            }

            if(note->pos().ticks() > end)
                drawSilence(p, x, y, w, h, note->pos().ticks() - end);

            if(note->pos().ticks() == start)
            {
                x = xstart;
            }
            else
            {
                start  = note->pos().ticks();
                xstart = x;
            }

            drawNote(p, x, y, w, h, note->key(), note->length().ticks());
            end = qMax(end, note->pos().ticks() + note->length().ticks());
            firstNote = false;
        }

        p.fillRect(QRect(x + 4, y - 25, w - x - 4, h), Qt::white);
        if(!firstNote)
            drawBar(p, x, y, w, h, 2);
        */
    }

    /*
    drawStave(p, x, y, w, h);
    drawBar(p, x, y, w, h, 3);
    drawTempo(p, x, y, w, h, 140);
    drawClef(p, x, y, w, h, 0);
    drawSignature(p, x, y, w, h, 3, 4);
    drawNote(p, x, y, w, h, 72, 48);
    drawNote(p, x, y, w, h, 75, 24);
    drawNote(p, x, y, w, h, 79, 24);
    drawBar(p, x, y, w, h, 0);
    x = w - 2 - 12;
    drawBar(p, x, y, w, h, 2);

    x = 2;
    y += h;
    drawStave(p, x, y, w, h);
    drawClef(p, x, y, w, h, 1);
    drawSignature(p, x, y, w, h, 4, 4);
    drawNote(p, x, y, w, h, 69, 96);
    drawNote(p, x, y, w, h, 67, 96);
    drawNote(p, x, y, w, h, 60, 96);
    drawNote(p, x, y, w, h, 81, 12);
    drawNote(p, x, y, w, h, 81, 12);
    drawNote(p, x, y, w, h, 72, 12);
    drawNote(p, x, y, w, h, 84, 12);
    // drawNote(p,x,y,w,h,63,24);
    // drawNote(p,x,y,w,h,63,24);

    x = 2;
    y += h;
    drawStave(p, x, y, w, h);
    drawClef(p, x, y, w, h, 2);
    drawSignature(p, x, y, w, h, 2, 2);
    drawNote(p, x, y, w, h, 69, 48);
    drawNote(p, x, y, w, h, 65, 48);
    drawNote(p, x, y, w, h, 62, 48);
    drawNote(p, x, y, w, h, 59, 48);

    x = 2;
    y += h;
    drawStave(p, x, y, w, h);
    drawClef(p, x, y, w, h, 3);
    drawSignature(p, x, y, w, h, 7, 8);
    drawNote(p, x, y, w, h, 60, 97);
    drawNote(p, x, y, w, h, 62, 145);
    drawNote(p, x, y, w, h, 70, 21);
    drawNote(p, x, y, w, h, 71, 24);
    drawNote(p, x, y, w, h, 72, 21);
    drawNote(p, x, y, w, h, 73, 18);
    drawNote(p, x, y, w, h, 74, 16);
    drawNote(p, x, y, w, h, 75, 12);
    drawNote(p, x, y, w, h, 76, 14);
    */
}
