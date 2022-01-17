import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.atomic.AtomicInteger;

public class TaskReduce implements Runnable {

    private final Dictionary dictionaries;
    private final ExecutorService tpe;
    private int maxLen = -1;
    private int count = 0;
    private final List<Result> result;
    private final StringBuilder finalResult = new StringBuilder();
    private AtomicInteger documentSize;
    private int noWords = 0;

    TaskReduce(Dictionary dictionary, ExecutorService tpe, List<Result> result, AtomicInteger documentSize) {
        this.dictionaries = dictionary;
        this.tpe = tpe;
        this.result = result;
        this.documentSize = documentSize;
    }

    @Override
    public void run() {
        // for each entry in dictionary's list
        // count the number of words by adding frequencies
        // and find the maximal length
        for (Dictionary.Entry e : dictionaries.getEntries()) {
            noWords += e.getFrequency();
            if (e.getLength() > maxLen) {
                maxLen = e.getLength();
                count = e.getFrequency();
            } else if (e.getLength() == maxLen) {
                count += e.getFrequency();
            }
        }
        // compute rang by the rule
        computeRang(maxLen, dictionaries);

        // compute the partial string
        // i use a class to save the rang separately for the final sorting
        String[] aux = dictionaries.getInputFile().split("/");
        finalResult.append(aux[2]);
        finalResult.append(",");
        finalResult.append(String.format("%.2f", dictionaries.getRang()));
        finalResult.append(",");
        finalResult.append(maxLen);
        finalResult.append(",");
        finalResult.append(count);
        result.add(new Result(dictionaries.getRang(), finalResult.toString()));
        documentSize.decrementAndGet();
        if (documentSize.get() == 0)
            tpe.shutdown();
    }


    public void computeRang(int maxLen, Dictionary dictionary) {
        int[] fib = new int[maxLen + 3];
        float sum = 0;
        fib[0] = 0;
        fib[1] = 1;

        for (Dictionary.Entry e : dictionary.getEntries()) {
            for (int i = 2; i <= maxLen + 1; i++) {
                fib[i] = fib[i - 1] + fib[i - 2];
                if (i == e.getLength() + 1) {
                    sum += fib[i] * e.getFrequency();
                    break;
                }
            }
        }
        dictionary.setRang(sum / noWords);
    }
}
