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

    void save(std::ofstream &f)
    {
        f << myIndex << " "
          << myfVertical << " "
          << myText << "\n"
          << myClue << "\n";
    }
};

class cCrossWord
{
public:
    int myDimension;
    std::vector<int> myBlack;
    std::vector<cWord> myWord;          ///< words placed in crossword


    cCrossWord()
        : myDimension(9), mySelected(-1)
    {
    }
    void clear();

    void draw(PAINTSTRUCT &ps);
    std::string textClues();
    void listSuggestions( wex::list& lsSugs );

    void add(const cWord &word);
    void addSuggestion(const cWord &word);
    cWord& suggestion( int i );

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
    int mouse2index(wex::sMouse m);

private:
    int xinc;
    int yinc;
    int mySelected;
    std::vector<cWord> mySuggestions;   ///< suggested words

    void AssignClueNumber();

};
