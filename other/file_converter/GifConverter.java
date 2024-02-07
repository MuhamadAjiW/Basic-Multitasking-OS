import java.util.ArrayList;

import javax.imageio.ImageIO;
import javax.imageio.ImageReader;
import javax.imageio.stream.ImageInputStream;

import java.awt.Color;
import java.awt.image.BufferedImage;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;

public class GifConverter {
    public static void main(String[] args) {
        ArrayList<BufferedImage> frames = new ArrayList<BufferedImage>();

        try{
            ImageReader reader = (ImageReader) ImageIO.getImageReadersByFormatName("gif").next();
            ImageInputStream stream = ImageIO.createImageInputStream(new File("assets/chuu.gif"));
            reader.setInput(stream);
            int count = reader.getNumImages(true);
            for (int i = 0; i < count; i++) {
                frames.add(reader.read(i));
            }
        } catch(Exception e){
            System.out.println("Error: " + e);
        }

        if (frames.isEmpty()){
            return;
        }
        
        //keperluan keperluan imageconverter
        int width = frames.get(0).getWidth();
        int height = frames.get(0).getHeight();
        int mod = 1;            //mod yang udah disebut
        int leninency = 5;      //leninency yang udah disebut
        int reswidth = width / mod;
        int resheight = height / mod;
        int colorCombined = reswidth * 3;

        int[][] pixels = new int[resheight][colorCombined];
        int[][] buffer = new int[resheight][reswidth];
        int colorcount = 0;
        int[][] colors = new int[256][3];
        
        try{
            FileOutputStream fos = new FileOutputStream("output/result_gif.bin");

            // nulis resolusi
            // nulis resolusi
            fos.write(String.valueOf(reswidth).getBytes());
            fos.write('\n');
            fos.write(String.valueOf(resheight).getBytes());
            fos.write('\n');
            fos.write(String.valueOf(frames.size()).getBytes());
            fos.write('\n');
            // fos.write('\n');

            for(int frame = 0; frame < frames.size(); frame++){
                for(int y = 0; y < resheight; y++){
                    for(int x = 0; x < reswidth; x++){
                        // baca gambar per pixel
                        Color color = new Color(frames.get(frame).getRGB(x * mod,y * mod));
                        
                        int loc = x*3;
                        
                        // convert ke 18 bit color per value
                        pixels[y][loc] = (color.getRed() * 63) / 255;
                        pixels[y][loc + 1] = (color.getGreen() * 63) / 255;
                        pixels[y][loc + 2] = (color.getBlue() * 63) / 255;
                        
                        if(colorcount == 0){
                            // inisiasi palette
                            colors[colorcount][0] = pixels[y][loc];
                            colors[colorcount][1] = pixels[y][loc + 1];
                            colors[colorcount][2] = pixels[y][loc + 2];
                            colorcount++;
                        }
                        else{
                            boolean found = false;
                            int i = 0;
                            for(i = 0; i < colorcount; i++){
                                // nyari di palette yang udah ada
                                if((colors[i][0] - leninency <= pixels[y][loc]  && colors[i][0] + leninency >= pixels[y][loc]) &&
                                (colors[i][1] - leninency <= pixels[y][loc + 1] && colors[i][1] + leninency >= pixels[y][loc + 1]) &&
                                (colors[i][2] - leninency <= pixels[y][loc + 2] && colors[i][2] + leninency >= pixels[y][loc + 2]))
                                {
                                    found = true;
                                    break;
                                }
                            }
                            
                            if(!found){
                                // nambah palette
                                colors[colorcount][0] = pixels[y][loc];
                                colors[colorcount][1] = pixels[y][loc + 1];
                                colors[colorcount][2] = pixels[y][loc + 2];
                                buffer[y][x] = colorcount;
                                
                                colorcount++;
                            }
                            else{
                                // ngisi bitmap
                                buffer[y][x] = i;
                            }
                        }
                        
                    }
                }

                // nulis bitmap
                for(int y = 0; y < resheight; y++){
                    for(int x = 0; x < reswidth; x++){
                        fos.write((int)(255 - buffer[y][x]));
                        // fos.write((255 - buffer[y][x]) + ((x == reswidth - 1 && y == resheight - 1) ? "" : ", "));
                    }
                    // fos.write('\n');
                }
                // fos.write('\n');
            }
            fos.write('\n');

            // nulis palette
            fos.write(colorcount);
            fos.write('\n');

            for(int y = colorcount - 1; y >= 0; y--){
                // if ((255 - y) % 4 == 0){
                //     fos.write('\n');
                // }
                for(int x = 0; x < 3; x++){
                    fos.write((int)colors[y][x]);
                    // fos.write(colors[y][x] + ((x == 2 && y == 0) ? "" : ", "));
                }
                // fos.write('\n');
            }
            fos.write('\n');
            fos.write('.');
            
            // fos.newLine();
            // fos.newLine();
            // fos.write("Palette");
            // fos.newLine();

            // // nulis palette
            // int counter = 0;
            // for(int y = 255 - bgcolor_offsets; y >= 0; y--){
            //     if(counter == 4){
            //         fos.newLine();
            //         counter = 0;
            //     }
            //     for(int x = 0; x < 3; x++){
            //         fos.write(colors[y][x] + ", ");
            //     }
            //     counter++;
            // }
            
            // for(int y = bgcolor_offsets; y > 0; y--){
            //     if(counter == 4){
            //         fos.newLine();
            //         counter = 0;
            //     }
            //     for(int x = 0; x < 3; x++){
            //         fos.write('0' + ((x == 2 && y == 0) ? "" : ", "));
            //     }
            //     counter++;
            // }
            
            fos.flush();
            fos.close();
            
            System.out.println("GIF conversion successful");
        } catch(Exception e){
            System.out.println("Error: " + e);
        }
    }
}
