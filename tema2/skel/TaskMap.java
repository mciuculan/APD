import java.util.*;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.atomic.AtomicInteger;

public class TaskMap implements Runnable {
    private final String documentName;
    private int offset; // start point
    private int lastPoint; // ending point
    private boolean finish = false;
    private final HashMap<String, Integer> words; // list of words and frequencies
    private final String toBeParsed; // string to be read and parsed
    private final AtomicInteger fragments;
    private final ExecutorService executorService;
    ConcurrentLinkedQueue<Dictionary> dictionaries; // partial solution
    String separators = ";:/?~\\.,><`[]{}()!@#$%^&-_+'=*\"| " + "\t" + "\r" + "\n";

    public TaskMap(String documentName, int offset, int dimension, String toBeParsed, AtomicInteger fragments,
                   ExecutorService executorService, ConcurrentLinkedQueue<Dictionary> dictionaries) {
        this.documentName = documentName;
        this.offset = offset;
        lastPoint = offset + dimension - 1;
        this.toBeParsed = toBeParsed;
        this.fragments = fragments;
        this.executorService = executorService;
        this.dictionaries = dictionaries;
        words = new HashMap<>();
    }

    // checks if the first word is full
    // true if it is
    private boolean checkFirst(char[] text) {
        // if offset is 0 means it's the beginning of the file
        if (offset == 0)
            return true;

        //if the first of the character before the start are separators,
        // it's the beginning of the word
        for (char c : separators.toCharArray())
            if (text[offset - 1] == c)
                return true;

        for (char c : separators.toCharArray())
            if (text[offset] == c)
                return true;
        return false;
    }

    // checks if the last word is full
    // true if it is
    private boolean checkLast(char[] text) {
        // if the last point is the length of the file
        // the word is full
        if (lastPoint == text.length - 1)
            return true;

        // if the last point or the next from the last point
        // is a separator, the word is full
        for (char c : separators.toCharArray())
            if (text[lastPoint + 1] == c)
                return true;

        for (char c : separators.toCharArray())
            if (text[lastPoint] == c)
                return true;
        return false;
    }

    // modifies the beginning of the fragment until it finds a separator
    private void newOffset(char[] text) {
        while ((offset < text.length) && separators.indexOf(text[offset]) < 0)
            offset++;
    }

    // modifies the ending of the file until it finds a separator
    private void newDimension(char[] text) {
        while ((lastPoint < text.length) && separators.indexOf(text[lastPoint]) < 0)
            lastPoint++;
    }

    @Override
    public void run() {
        if (toBeParsed.length() - 1 > offset) {
            if (lastPoint >= toBeParsed.length() - 1)
                lastPoint = toBeParsed.length() - 2;

            char[] text = toBeParsed.toCharArray();

            if (!checkFirst(text)) {
                newOffset(text);
            }

            if (!checkLast(text)) {
                newDimension(text);
            }

            if ((offset + 1 == text.length - 1) && (separators.indexOf(text[offset]) < 0))
                finish = true;

            if (!finish) {
                // a vector of words
                String[] wordsAux = toBeParsed.substring(offset, lastPoint)
                        .split("[;:/?~.,><`\\]{}()!@#$%^&\\-_+'=*\"| \\t\\r\\n]+");

                // added to hashmap only if it doesn't exist
                for (String s : wordsAux) {
                    if (!words.containsKey(s))
                        words.put(s, 1);
                    else
                        words.put(s, words.get(s) + 1);
                }
            }

            dictionaries.add(getDictionary());

            // if i finished the last fragment, i shut down the executor service
            fragments.decrementAndGet();
            if (fragments.get() == 0)
                executorService.shutdown();
        }
    }

    public HashMap<String, Integer> getWords() {
        return words;
    }

    // add to a treemap so they are ordered decreasingly
    private TreeMap<String, Integer> getCorrectOrder() {
        TreeMap<String, Integer> treeMap = new TreeMap<>((s1, s2) -> {
            if (s1.length() > s2.length())
                return -1;
            else if (s1.length() < s2.length())
                return  1;
            else return s1.compareTo(s2);
        });
        treeMap.putAll(words);
        return treeMap;
    }

    public Dictionary getDictionary() {
        // basically, if the words list is empty
        if (getCorrectOrder().isEmpty())
            return null;

        List<Dictionary.Entry> entries = new ArrayList<>();
        Dictionary.Entry entry;
        boolean alreadyExists = false;
        int i = 0;
        // for each word in set, if the word is not null
        for (String s : getCorrectOrder().keySet()) {
            if (s.length() > 0) {
                // if i have entries
                if (!entries.isEmpty()) {
                    // check if the word exists in the list of entries
                    for (Dictionary.Entry e : entries) {
                        if (e.getLength() == s.length()) {
                            alreadyExists = true;
                            break;
                        }
                        i++;
                    }
                }
                // if it doesn't exist, i add it to the list
                if (!alreadyExists) {
                    entry = new Dictionary.Entry(s.length(), getCorrectOrder().get(s));
                    entries.add(entry);
                } else {
                    // else, i add it's frequency to the existing entry
                    entry = new Dictionary.Entry(s.length(), entries.get(i).getFrequency() + words.get(s));
                    entries.set(i, entry);
                }
            }
            i = 0;
            alreadyExists = false;
        }
        List<String> longest = new ArrayList<>();
        // used to get the longest word
        longest.add(getCorrectOrder().firstKey());
        return new Dictionary(documentName, entries, longest);
    }
}
