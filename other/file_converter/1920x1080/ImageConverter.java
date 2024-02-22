import java.awt.Color;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import javax.imageio.ImageIO;


public class ImageConverter {
        public static void main(String[] args) {
        try{
            BufferedImage image = ImageIO.read(new File("assets/waifu1.jpg"));     //kalo mau ganti nama file di sini
            
            int width = image.getWidth();
            int height = image.getHeight();
    
            int[][] buffer = new int[height][width];
    
            for(int y = 0; y < height; y++){
                for(int x = 0; x < width; x++){
                    // baca gambar per pixel
                    Color color = new Color(image.getRGB(x,y));
                    buffer[y][x] = color.getRGB();
                }
            }

            FileOutputStream fos = new FileOutputStream("output/result.bin");

            // nulis resolusi
            fos.write(String.valueOf(width).getBytes());
            fos.write('\n');
            fos.write(String.valueOf(height).getBytes());
            fos.write('\n');
            // fos.write('\n');

            // nulis bitmap
            byte[] bytes = new byte[4];
            for(int y = 0; y < height; y++){
                for(int x = 0; x < width; x++){
                    bytes[0] = (byte) (buffer[y][x] >> 24);
                    bytes[1] = (byte) (buffer[y][x] >> 16);
                    bytes[2] = (byte) (buffer[y][x] >> 8);
                    bytes[3] = (byte) buffer[y][x];
                    fos.write(bytes[3]);
                    fos.write(bytes[2]);
                    fos.write(bytes[1]);
                    fos.write(bytes[0]);
                }
            }
            fos.write('\n');

            fos.write('.');

            fos.flush();
            fos.close();

            System.out.println("Image conversion successful");
        } catch(IOException e){
            System.out.println("Error: " + e);
        }
    }
}
