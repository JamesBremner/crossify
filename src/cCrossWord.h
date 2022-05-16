class cWord
{
public:
    int myIndex;
    bool myfVertical;
    std::string myText;
    std::string myClue;
    int myClueNo;

    cWord()
    {
    }

    cWord(
        const std::string &line,
        const std::string &clueline);

    std::string dump()
    {
        std::stringstream ss;
        ss << myIndex << " "
           << myfVertical << " "
           << myText << "\n"
           << myClue << "\n";
        return ss.str();
    }

    void save(std::ofstream &f)
    {
        f << dump();
    }
};

class cCrossWord
{
public:
    int myDimension;
    std::vector<cWord> myWord; ///< words placed in crossword

    cCrossWord()
        : myDimension(10), mySelected(-1), myfWords(true)
    {
    }
    void clear();
    void clearLayout();
    void toggleWords();

    void draw(PAINTSTRUCT &ps);
    std::string textClues();
    void listSuggestions(wex::list &lsSugs);

    bool add(cWord &word);
    void addSuggestion(const cWord &word);
    cWord &suggestion(int i);

    void select(int i);
    int select() const
    {
        return mySelected;
    }

    void save(const std::string &fname);
    void read(const std::string &fname);

    cWord &findWord(int i, bool vert = false);

    std::pair<int, int> index2colrow(int i);
    int colrow2index(std::pair<int, int> colrow);
    int pixel2index(int x, int y);

    void autoAdd(bool myVertical = false);

private:
    int xinc;
    int yinc;
    int mySelected;
    std::vector<cWord> mySuggestions; ///< suggested words
    std::vector<char> myFill;
    bool myfWords;

    void AssignClueNumber();
    bool check( const cWord& word );
};
