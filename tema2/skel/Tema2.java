import java.io.*;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.*;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicInteger;

class Result {
    private final float rang;
    private final String toBePrinted;

    public Result(float rang, String toBePrinted) {
        this.rang = rang;
        this.toBePrinted = toBePrinted;
    }

    public float getRang() {
        return rang;
    }

    public String getToBePrinted() {
        return toBePrinted;
    }
}

public class Tema2 {

    // reads files byte by byte
    private static String read(String documentName) {
        String data = "";
        try {
            data = new String(Files.readAllBytes(Paths.get(documentName)));
        } catch (IOException e) {
            e.printStackTrace();
        }
        return data;
    }

    public static void main(String[] args) {
        if (args.length < 3) {
            System.err.println("Usage: Tema2 <workers> <in_file> <out_file>");
            return;
        }
        int workers = Integer.parseInt(args[0]);
        String input_file = args[1];
        String output_file = args[2];

        // read input file
        Reader reader = new Reader(input_file);
        String[] filesToRead = reader.readFile();
        int noFiles = reader.getNoFiles();
        int fragmentDimension = reader.getDimension();

        // queue in witch each worker will add partial solutions
        ConcurrentLinkedQueue<Dictionary> dictionaries = new ConcurrentLinkedQueue<>();
        // final combined dictionary
        List<Dictionary> dictionaryList = new ArrayList<>();

        // for each file, start map
        for (int file = 0; file < noFiles; file++) {
            // read the file
            String toBeParsed = read(filesToRead[file]);

            // start map
            // create a new pool for each file
            ExecutorService executorService = Executors.newFixedThreadPool(workers);

            // keeps count of number of fragments that need to be parsed
            AtomicInteger fragments = new AtomicInteger(toBeParsed.length() / fragmentDimension);

            for (int i = 0; i < toBeParsed.length() && !executorService.isTerminated(); i += fragmentDimension) {
                executorService.submit(new TaskMap(filesToRead[file], i, fragmentDimension, toBeParsed, fragments, executorService, dictionaries));
            }

            // barrier; we don't go to the next file until we finish this one
            try {
                while (!executorService.isTerminated());
            } catch (Exception e) {
                System.out.println(e.getMessage());
            }
        }

        // create full dictionary
        for (Dictionary dictionary : dictionaries) {
            // if the final list is empty, add dictionary
            if (dictionaryList.isEmpty())
                dictionaryList.add(dictionary);
            else {
                boolean exists = false;
                // already have some dictionaries in the list
                for (Dictionary d : dictionaryList) {
                    // if i find a previous added file, i add the new entries
                    if (d.getInputFile().equals(dictionary.getInputFile())) {
                        // if the entry already exists, i only add the frequency to it
                        for (Dictionary.Entry e : dictionary.getEntries()) {
                            boolean entryExists = false;
                            int i = 0;
                            for (Dictionary.Entry entry : d.getEntries()) {
                                if (e.getLength() == entry.getLength()) {
                                    entryExists = true;
                                    d.getEntries().set(i, new Dictionary.Entry(e.getLength(), e.getFrequency() + entry.getFrequency()));
                                }
                                i++;
                            }
                            if (!entryExists && e.getLength() != 0)
                                d.addEntries(e);
                        }
                        d.addWord(dictionary.getLongestWord().get(0));
                        exists = true;
                        break;
                    }
                }
                // if i couldn't find the file, i add the new dictionary to the list
                if (!exists) {
                    dictionaryList.add(dictionary);
                }
            }
        }

        // start reduce
        ExecutorService tpe = Executors.newFixedThreadPool(workers);
        List<Result> results = Collections.synchronizedList(new ArrayList<>());
        AtomicInteger size = new AtomicInteger(dictionaryList.size());
        // for each dictionary (created for each file), we compute the results
        for (Dictionary dictionary : dictionaryList) {
            tpe.submit(new TaskReduce(dictionary, tpe, results, size));
        }

        try {
            while (!tpe.isTerminated());
        } catch (Exception e) {
            System.out.println(e.getMessage());
        }

        results.sort((result1, result2) -> {
            if (result2.getRang() > result1.getRang())
                return 1;
            else if (result2.getRang() == result1.getRang())
                return 0;
            else return -1;
        });

        // write output to file

        try {
            new FileWriter(output_file, false).close();
            BufferedWriter writer = new BufferedWriter(new FileWriter(output_file, true));

            for (Result r : results) {
                writer.append(r.getToBePrinted());
                writer.append('\n');
            }
            writer.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
