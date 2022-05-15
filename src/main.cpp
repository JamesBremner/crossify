#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <wex.h>

#include "cCrossWord.h"

// https://github.com/AyeshJayasekara/English-Dictionary-SQLite

cCrossWord theCrossWord;

std::vector<std::string> ParseSpaceDelimited(
    const std::string &l)
{
    std::vector<std::string> token;
    std::stringstream sst(l);
    std::string a;
    while (getline(sst, a, ' '))
        token.push_back(a);

    token.erase(
        remove_if(
            token.begin(),
            token.end(),
            [](std::string t)
            {
                return (t.empty());
            }),
        token.end());

    return token;
}

cWord::cWord(
    const std::string &line,
    const std::string &clueline)
{
    auto toks = ParseSpaceDelimited(line);
    myIndex = atoi(toks[0].c_str());
    myfVertical = (toks[1] == "1");
    myText = toks[2];
    myClue = clueline;
}

void cCrossWord::clear()
{
    myBlack.clear();
    myWord.clear();
    mySuggestions.clear();
    mySelected = -1;
}

void cCrossWord::clearLayout()
{
    myWord.clear();
}

void cCrossWord::toggleWords()
{
    myfWords = !myfWords;
}

void cCrossWord::select(int i)
{
    mySelected = i;
}

cWord &cCrossWord::findWord(int i, bool vert)
{
    for (int k = 0; k < myWord.size(); k++)
    {
        if (myWord[k].myIndex == i && myWord[k].myfVertical == vert)
            return myWord[k];
    }
    static cWord none;
    return none;
}

void cCrossWord::AssignClueNumber()
{
    int clue = 1;
    for (int i = 0; i < myDimension * myDimension; i++)
    {
        auto &h = findWord(i);
        if (!h.myText.empty())
            h.myClueNo = clue;
        auto &v = findWord(i, true);
        if (!v.myText.empty())
            v.myClueNo = clue;
        if (!(h.myText.empty() && v.myText.empty()))
        {
            clue++;
        }
    }
}

void cCrossWord::draw(PAINTSTRUCT &ps)
{
    myFill.resize(myDimension * myDimension, '*');

    wex::shapes S(ps);
    yinc = (ps.rcPaint.bottom - 40) / myDimension;
    int x1 = 20;
    int y1 = 20;
    int x2 = 20 + myDimension * yinc;
    int y2 = 20;
    S.line({x1, y1, x2, y2});

    for (int k = 0; k < myDimension; k++)
    {
        y1 += yinc;
        y2 += yinc;
        S.line({x1, y1, x2, y2});
    }
    xinc = yinc;
    x1 = 20;
    y1 = 20;
    x2 = 20;
    y2 = ps.rcPaint.bottom - 20;
    S.line({x1, y1, x2, y2});
    for (int k = 0; k < myDimension; k++)
    {
        x1 += xinc;
        x2 += xinc;
        S.line({x1, y1, x2, y2});
    }
    S.color(0xD0E040);
    S.fill();
    for (int i : myBlack)
    {
        auto colrow = index2colrow(i);
        x1 = 21 + colrow.first * xinc;
        y1 = 21 + colrow.second * yinc;
        S.rectangle({x1, y1, xinc - 1, yinc - 1});
    }

    S.textHeight(30);
    S.bgcolor(0xFFFFFF);
    for (auto &w : myWord)
    {
        int inc = 1;
        if (w.myfVertical)
            inc = myDimension;
        for (int c = 0; c < w.myText.length(); c++)
        {
            int letterindex = w.myIndex + c * inc;
            myFill[letterindex] = w.myText[c];
            auto colrow = index2colrow(letterindex);
            x1 = colrow.first * xinc;
            y1 = colrow.second * yinc;
            S.color(0xFFFFFF);
            S.rectangle({x1 + 21, y1 + 21, xinc - 1, yinc - 1});
            if (myfWords)
            {
                S.color(0);
                S.text(w.myText.substr(c, 1), {x1 + 40, y1 + 30});
            }
        }
    }

    AssignClueNumber();
    S.color(0x000000);
    S.bgcolor(0xFFFFFF);
    S.textHeight(15);
    for (auto &w : myWord)
    {
        auto colrow = index2colrow(w.myIndex);
        x1 = 22 + colrow.first * xinc;
        y1 = 21 + colrow.second * xinc;
        S.text(std::to_string(w.myClueNo), {x1, y1});
    }

    if (mySelected >= 0)
    {
        S.color(0x0000FF);
        S.fill(false);
        auto colrow = index2colrow(mySelected);
        x1 = 21 + colrow.first * xinc;
        y1 = 21 + colrow.second * yinc;
        S.rectangle({x1, y1, xinc - 1, yinc - 1});
    }
}

std::pair<int, int> cCrossWord::index2colrow(int i)
{
    std::pair<int, int> colrow;
    colrow.second = i / myDimension;
    colrow.first = i - colrow.second * myDimension;
    return colrow;
}
int cCrossWord::colrow2index(std::pair<int, int> colrow)
{
    return colrow.second * theCrossWord.myDimension + colrow.first;
}
int cCrossWord::pixel2index(int x, int y)
{
    if (x > 20 + myDimension * xinc ||
        y > 20 * myDimension * yinc)
        return -1;
    return colrow2index(
        std::make_pair(
            (x - 20) / xinc,
            (y - 20) / yinc));
}

void cCrossWord::add(const cWord &word)
{
    auto &oldword = findWord(word.myIndex, word.myfVertical);
    if (oldword.myText.empty())
    {
        myWord.push_back(word);
        return;
    }
    oldword = word;

    // int i;
    // if (word.myfVertical)
    // {
    //     i = word.myIndex + word.myText.length() * myDimension;
    // }
    // else
    // {
    //     i = word.myIndex + word.myText.length();
    // }
    // myBlack.push_back(i);
    // myContents[i] = '*';
}

void cCrossWord::addSuggestion(const cWord &word)
{
    mySuggestions.push_back(word);
}
cWord &cCrossWord::suggestion(int i)
{
    return mySuggestions[i];
}

void cCrossWord::save(const std::string &fname)
{
    std::ofstream f(fname);
    for (auto &w : myWord)
    {
        w.save(f);
    }
    f << "CWSUGGESTIONS\n";
    for (auto &w : mySuggestions)
    {
        w.save(f);
    }
}
void cCrossWord::read(const std::string &fname)
{
    clear();
    bool fSugs = false;
    std::ifstream f(fname);
    std::string line, clueline;
    while (getline(f, line))
    {
        if (line == "CWSUGGESTIONS")
        {
            fSugs = true;
            continue;
        }
        getline(f, clueline);
        cWord word(line, clueline);
        if (!fSugs)
            add(word);
        else
            addSuggestion(word);
    }
}

std::string cCrossWord::textClues()
{
    std::map<int, std::string> mp;
    for (auto &w : myWord)
    {
        if (w.myfVertical)
            continue;
        mp.insert(std::make_pair(w.myClueNo, w.myClue));
    }
    std::stringstream ss;
    ss << "Across\n";
    for (auto it : mp)
    {
        ss << it.first << " " << it.second << "\n";
    }
    mp.clear();
    for (auto &w : myWord)
    {
        if (!w.myfVertical)
            continue;
        mp.insert(std::make_pair(w.myClueNo, w.myClue));
    }
    ss << "\nDown\n";
    for (auto it : mp)
    {
        ss << it.first << " " << it.second << "\n";
    }
    return ss.str();
}

void cCrossWord::listSuggestions(wex::list &lsSugs)
{
    lsSugs.clear();
    for (auto &w : mySuggestions)
    {
        lsSugs.add(w.myText);
    }
    lsSugs.update();
}

class cGUI
{
public:
    cGUI();

private:
    wex::gui &fm;
    wex::tabbed &tabs;
    wex::panel &plEdit;
    wex::panel &plSug;
    wex::panel &plClues;
    wex::editbox &wordbox;
    wex::label &wordLabel;
    wex::button &bnAdd;
    wex::radiobutton &bnHoriz;
    wex::radiobutton &bnVert;
    wex::editbox &cluebox;
    wex::label &clueLabel;

    wex::editbox &sgwordbox;
    wex::button &bnsgAdd;
    wex::editbox &sgcluebox;
    wex::list &lsSugs;
    wex::label &lbCluesAcross;
    wex::label &lbCluesDown;

    void ConstructMenu();
    void RegisterEventHandlers();
};

cGUI::cGUI()
    : fm(wex::maker::make()),
      tabs(wex::maker::make<wex::tabbed>(fm)),
      plEdit(wex::maker::make<wex::panel>(tabs)),
      plSug(wex::maker::make<wex::panel>(tabs)),
      plClues(wex::maker::make<wex::panel>(tabs)),
      wordLabel(wex::maker::make<wex::label>(plEdit)),
      wordbox(wex::maker::make<wex::editbox>(plEdit)),
       bnAdd(wex::maker::make<wex::button>(plEdit)),
        bnHoriz(wex::maker::make<wex::radiobutton>(plEdit)),
         bnVert(wex::maker::make<wex::radiobutton>(plEdit)),
         clueLabel(wex::maker::make<wex::label>(plEdit)),
          cluebox(wex::maker::make<wex::editbox>(plEdit)),

      sgwordbox(wex::maker::make<wex::editbox>(plSug)),
      bnsgAdd(wex::maker::make<wex::button>(plSug)), sgcluebox(wex::maker::make<wex::editbox>(plSug)), lsSugs(wex::maker::make<wex::list>(plSug)), lbCluesAcross(wex::maker::make<wex::label>(plClues)), lbCluesDown(wex::maker::make<wex::label>(plClues))
{
    fm.move({50, 50, 1000, 500});
    fm.text("Crossify");

    ConstructMenu();

    tabs.move(450, 20, 500, 450);
    tabs.tabWidth(150);
    tabs.add("EDIT", plEdit);
    //tabs.add("SUGGESTIONS", plSug);
    tabs.add("CLUES", plClues);

    wordLabel.move(0,35, 50, 30 );
    wordLabel.text("Word");
    wordbox.move(100, 30, 300, 30);
    wordbox.text("");
    bnAdd.move(100, 70, 50, 30);
    bnAdd.text("ADD");
    bnHoriz.move(160, 70, 50, 30);
    bnHoriz.text("Horiz");
    bnVert.move(240, 70, 50, 30);
    bnVert.text("Vert");
    clueLabel.move(0,135, 30, 30 );
    clueLabel.text("Clue");
    cluebox.move(50, 130, 400, 30);
    cluebox.text("");

    sgwordbox.move(100, 30, 300, 30);
    sgwordbox.text("");
    bnsgAdd.move(100, 70, 50, 30);
    bnsgAdd.text("ADD");
    sgcluebox.move(100, 130, 300, 30);
    sgcluebox.text("");
    lsSugs.move(50, 200, 400, 300);

    lbCluesAcross.move(0, 50, 450, 400);
    lbCluesDown.move(510, 50, 450, 400);

    RegisterEventHandlers();

    // show and run
    fm.show();
    tabs.select(0);
    fm.run();
}

void cGUI::ConstructMenu()
{
    wex::menubar mb(fm);
    wex::menu m(fm);
    m.append("New", [&](const std::string &title)
             { theCrossWord.clear();
             fm.update(); });
    m.append("Clear", [&](const std::string &title)
             { theCrossWord.clearLayout();
             fm.update(); });
    m.append("Words", [&](const std::string &title)
             { theCrossWord.toggleWords();
             fm.update(); });
    m.append("Save", [&](const std::string &title)
             { 
                 wex::filebox fb(fm);
                 auto fname = fb.save();
                 theCrossWord.save( fname ); });
    m.append("Open", [&](const std::string &title)
             { 
                 wex::filebox fb(fm);
                 auto fname = fb.open();
                 theCrossWord.read( fname );
                 theCrossWord.listSuggestions( lsSugs ); });
    mb.append("File", m);
}

void cGUI::RegisterEventHandlers()
{
    fm.events().draw([&](PAINTSTRUCT &ps)
                     { theCrossWord.draw(ps);
                     lbCluesAcross.text( theCrossWord.textClues() );
                     lbCluesDown.text( theCrossWord.textClues() ); });

    fm.events().mouseUp(
        [&]
        {
            // select grid cell under mouse cursor
            auto m = fm.getMouseStatus();
            theCrossWord.select(
                theCrossWord.pixel2index(m.x,m.y));
            
            // if clicked first letter of a word
            cWord &w = theCrossWord.findWord(
                theCrossWord.select(),
                bnVert.isChecked());
            if (!w.myText.empty())
            {
                // load word into editing boxes
                wordbox.text(w.myText);
                cluebox.text(w.myClue);
            }
            fm.update();
        });

    bnAdd.events().click(
        [&]
        {
            cWord word;
            word.myText = wordbox.text();
            word.myIndex = theCrossWord.select();
            word.myfVertical = bnVert.isChecked();
            word.myClue = cluebox.text();
            theCrossWord.add(word);
            fm.update();
        });
    bnsgAdd.events().click(
        [&]
        {
            if (sgwordbox.text().empty())
            {
                theCrossWord.autoAdd(bnVert.isChecked());
                fm.update();
                return;
            }
            cWord word;
            word.myText = sgwordbox.text();
            word.myClue = sgcluebox.text();
            theCrossWord.addSuggestion(word);
            theCrossWord.listSuggestions(lsSugs);
        });
    lsSugs.events().select(
        lsSugs.id(), [this]
        { 
            auto& word = theCrossWord.suggestion( lsSugs.selectedIndex() );
            word.myIndex = theCrossWord.select();
            word.myfVertical = bnVert.isChecked();
            theCrossWord.add(word);
            fm.update(); });
}
void cCrossWord::autoAdd(bool fVertical)
{
    for (auto &w : mySuggestions)
    {
        if (!findWord(w.myIndex).myText.empty())
            continue;
        if (!findWord(w.myIndex, true).myText.empty())
            continue;
        if (myFill[mySelected] != '*')
            if (myFill[mySelected] != w.myText[0])
                continue;
        w.myfVertical = fVertical;
        w.myIndex = mySelected;
        add(w);
        break;
    }
}
main()
{
    cGUI theGUI;
}
