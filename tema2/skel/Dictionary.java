import java.util.List;

public class Dictionary {
    static class Entry {
        private final int length;
        private int frequency;

        Entry (int length, int frequency) {
            this.length = length;
            this.frequency = frequency;
        }

        public int getLength() {
            return length;
        }

        public int getFrequency() {
            return frequency;
        }
    }

    private final String inputFile;
    private final List<Entry> entries;
    private final List<String> longestWord;
    private float rang;

    public Dictionary(String inputFile, List<Entry> entries, List<String> longestWord) {
        this.inputFile = inputFile;
        this.entries = entries;
        this.longestWord = longestWord;
    }

    public String getInputFile() {
        return inputFile;
    }

    public List<Entry> getEntries() {
        return entries;
    }

    public void addEntries(Entry e) {
        entries.add(e);
    }

    public void addWord(String word) {
        longestWord.add(word);
    }

    public List<String> getLongestWord() {
        return longestWord;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(inputFile);
        sb.append(";");
        sb.append("{");
        int i = 0;
        for (Entry e : entries) {
            sb.append(e.getLength());
            sb.append(": ");
            sb.append(e.getFrequency());
            i++;
            if (i < entries.size())
                sb.append(", ");
        }
        sb.append("}");
        sb.append(";");
        sb.append("(\"");
        sb.append(longestWord);
        sb.append("\")");
        return sb.toString();
    }

    public float getRang() {
        return rang;
    }

    public void setRang(float rang) {
        this.rang = rang;
    }
}
