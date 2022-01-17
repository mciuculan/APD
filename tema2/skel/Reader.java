import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;

// reads first inputs
public class Reader {
    private int dimension;
    private int noFiles;
    private String[] output;
    private final String fileName;

    public Reader(String fileName) {
        this.fileName = fileName;
    }

    public String[] readFile() {
        try {
            File file = new File(fileName);
            Scanner scanner = new Scanner(file);
            int i = 0, j = 0;
            while (scanner.hasNextLine()) {
                if (j == 0) {
                    dimension = Integer.parseInt(scanner.nextLine());
                }
                if (j == 1) {
                    noFiles = Integer.parseInt(scanner.nextLine());
                    output = new String[noFiles + 1];
                }
                if (j > 1) {
                    output[i] = scanner.nextLine();
                    if (scanner.hasNextLine())
                        i++;
                }
                j++;
            }
            scanner.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        return output;
    }

    public int getDimension() {
        return dimension;
    }

    public int getNoFiles() {
        return noFiles;
    }

    public String getFileName() {
        return fileName;
    }
}
