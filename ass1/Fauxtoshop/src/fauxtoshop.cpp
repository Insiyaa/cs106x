// This is the CPP file you will edit and turn in.
// Also remove these comments here and add your own.
// TODO: rewrite this comment

#include <iostream>
#include "console.h"
#include "gwindow.h"
#include "grid.h"
#include "simpio.h"
#include "strlib.h"
#include "gbufferedimage.h"
#include "gevents.h"
#include "math.h" //for sqrt and exp in the optional Gaussian kernel
#include "gmath.h" // for sinDegrees(), cosDegrees(), PI
using namespace std;

static const int    WHITE = 0xFFFFFF;
static const int    BLACK = 0x000000;
static const int    GREEN = 0x00FF00;

void     doFauxtoshop(GWindow &gw, GBufferedImage &img);

bool     openImageFromFilename(GBufferedImage& img, string filename);
bool 	 saveImageToFilename(const GBufferedImage &img, string filename);
void     getMouseClickLocation(int &row, int &col);
Vector<double> gaussKernelForRadius(int radius);


void Scatter(GBufferedImage& img, const Grid<int>& img_grid, int degree) {
    // create dup of grid and mutate img reference.

    Grid<int> duplicate = img_grid;

    int rows = img_grid.numRows();
    int cols = img_grid.numCols();

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int ran_r, ran_c;
            do {
                ran_r = randomInteger(-degree, degree);
                ran_c = randomInteger(-degree, degree);
            } while(r+ran_r < 0 || r+ran_r >= rows || c+ran_c < 0 || c+ran_c >= cols);

            duplicate[r][c] = img_grid[r+ran_r][c+ran_c];

        }
    }

    img.fromGrid(duplicate);
}

bool isEdge(GBufferedImage& img, const Grid<int>& img_grid, int threshold, int r, int c) {

    int r1, g1, b1, r2, g2, b2;

    GBufferedImage::getRedGreenBlue(img_grid[r][c], r1, g1, b1);
    for(int i=-1; i<=1; i++){
        for (int j=-1; j<=1; j++){
            if(img_grid.inBounds(r+i, c+j)){
                GBufferedImage::getRedGreenBlue(img_grid[r+i][c+j], r2, g2, b2);
                if(max(abs(r1-r2), max(abs(g1-g2), abs(b1-b2))) > threshold){
                    return true;
                }
            }

        }
    }
    return false;
}

void EdgeDetect(GBufferedImage& img,const Grid<int>& img_grid, int threshold) {
    Grid<int> duplicate =  img_grid;
    int rows = img_grid.numRows();
    int cols = img_grid.numCols();

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (isEdge(img, img_grid, threshold, r, c)) {
                duplicate[r][c] = BLACK;
            }
            else {
                duplicate[r][c] = WHITE;
            }

        }
    }

    img.fromGrid(duplicate);
}

bool isGreen(const Grid<int> stk_grid, int row, int col, int tol) {
    int r1, g1, b1, r2, g2, b2;
    GBufferedImage::getRedGreenBlue(GREEN, r1, g1, b1);
    GBufferedImage::getRedGreenBlue(stk_grid[row][col], r2, g2, b2);

    if (max(abs(r1-r2), max(abs(g1-g2), abs(b1-b2))) < tol) {
        return  true;
    }
    else {
        return false;
    }
}

void GreenScreen(GBufferedImage& img, const Grid<int>& img_grid, const Grid<int> stk_grid, int row, int col, int tol) {
    Grid<int> duplicate = img_grid;
    int rows = stk_grid.numRows();
    int cols = stk_grid.numCols();

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if(img_grid.inBounds(row+r, col+c)) {
                if(!isGreen(stk_grid, r, c, tol)) {
                    duplicate[row+r][col+c] = stk_grid[r][c];
                }
            }
        }
    }
    img.fromGrid(duplicate);
}

void Rotate(GBufferedImage& img, const Grid<int>& img_grid, int angle) {
    // create dup of grid and mutate img reference.

    Grid<int> duplicate = img_grid;

    int rows = img_grid.numRows();
    int cols = img_grid.numCols();

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {

            int row_r = -(c * sinDegrees(angle)) + (r + cosDegrees(angle)) + rows/2;
            int col_r = (c * cosDegrees(angle) + (r * sinDegrees(angle)));

            if(img_grid.inBounds(row_r, col_r)) {
                duplicate[r][c] = img_grid[row_r][col_r];
            } else {
                duplicate[r][c] = WHITE;
            }


        }
    }

    img.fromGrid(duplicate);
}
void GaussianBlur(GBufferedImage& img, const Grid<int>& img_grid, const Vector<double>& kernel) {
    // create dup of grid and mutate img reference.

    Grid<int> duplicate = img_grid;

    int rows = img_grid.numRows();
    int cols = img_grid.numCols();

    int len = kernel.size();

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            for (int k = 0; k < len; k++) {
                if(img_grid.inBounds(r+k, c)) {
                    duplicate [r][c] += img_grid[r+k][c] * kernel[k];
                }
            }

        }
    }

    img.fromGrid(duplicate);
}


/* STARTER CODE FUNCTION - DO NOT EDIT
 *
 * This main simply declares a GWindow and a GBufferedImage for use
 * throughout the program. By asking you not to edit this function,
 * we are enforcing that the GWindow have a lifespan that spans the
 * entire duration of execution (trying to have more than one GWindow,
 * and/or GWindow(s) that go in and out of scope, can cause program
 * crashes).
 */
int main() {
    GWindow gw;
    gw.setTitle("Fauxtoshop");
    gw.setVisible(true);
    GBufferedImage img;
    doFauxtoshop(gw, img);
    return 0;
}

/* This is yours to edit. Depending on how you approach your problem
 * decomposition, you will want to rewrite some of these lines, move
 * them inside loops, or move them inside helper functions, etc.
 *
 * TODO: rewrite this comment.
 */
void doFauxtoshop(GWindow &gw, GBufferedImage &img) {

    cout << "Welcome to Fauxtoshop!" << endl;
    openImageFromFilename(img, "kitten.jpg");
    gw.setCanvasSize(img.getWidth(), img.getHeight());
    gw.add(&img,0,0);

    while(true) {
        string filename = getLine("Enter the name of file to open(blank to quit):");
        if (filename == ""){
            break;
        }
        cout << "Opening image, make take a while..." << endl;
        bool status = openImageFromFilename(img, filename);
        if (!status) {
            cout << "Image doesn't exist, try again!" << endl;
        }
        else {
            Grid<int> image_grid = img.toGrid();
            gw.setCanvasSize(img.getWidth(), img.getHeight());
            gw.add(&img,0,0);



            cout << "Which image filter would you like to apply?" << endl;
            cout << "1 - Scatter" << endl;
            cout << "2 - Edge detection" << endl;
            cout << "3 - Green screen with another image" << endl;
            cout << "4 - Compare with another image" << endl;
            cout << "5 - Rotation" << endl;
            cout << "6 - Gaussian blur" << endl;

            int choice;
            do {
                choice = getInteger("Your choice: ");
            }while(choice < 0 || choice > 6);

            switch (choice) {
            case 1:
                // Scatter
                int degree;
                do {
                    degree = getInteger("Enter degree of scatter (1-100): ");
                } while (degree > 100 || degree < 1);

                Scatter(img, image_grid, degree);
                break;
            case 2:

                //Edge Detection

                int threshold;
                do {
                    threshold = getInteger("Enter degree of scatter (1-100): ");
                } while (threshold < 1);

                EdgeDetect(img, image_grid, threshold);

                break;
            case 3:
            {
                // Green Screen
                GBufferedImage sticker;

                // 1. Prompt for sticker image
                while (true) {
                    string filename = getLine("Enter the name of file to superimpose: ");

                    bool stat = openImageFromFilename(sticker, filename);
                    if (!stat) {
                        cout << "Image doesn't exist, try again!" << endl;
                    }
                    else {
                        break;
                    }
                }

                Grid<int> stk_grid = sticker.toGrid();

                // 2. Prompt for tolerance for pure green (1 - 100)
                int tol;
                do {
                    tol = getInteger("Enter tolerance (1-100): ");
                } while (tol > 100 || tol < 1);

                // 3. Location of sticker. Non-negative "(row, col)". Place upper left corner of sticker. Or no input and mouse click.
                int row, col;
                char a;
                do {
                    string loc = getLine("Enter location to place the sticker img as \"(row,col)\" (or blank to use mouse):");
                if (loc != "") {
                    stringstream ss(loc);
                    ss >> a >> row >> a >> col;
                }
                else {
                    getMouseClickLocation(row, col);
                }
                    cout << endl << "Get location: (" << row << ", " << col << ")." << endl;
                } while (row < 0 || col < 0);

                // 4. Place sticker. Copy onto background if green value greater than threshold.
                GreenScreen(img, image_grid, stk_grid, row, col, tol);

                break;
            }
            case 4:
            {
                GBufferedImage img2;
                cout << "Now choose another image file to compare with." << endl;

                while (true) {
                    string filename = getLine("Enter the name of file to superimpose: ");

                    bool stat = openImageFromFilename(img2, filename);
                    if (!stat) {
                        cout << "Image doesn't exist, try again!" << endl;
                    }
                    else {
                        break;
                    }
                }

                int count = img.countDiffPixels(img2);
                if (count == 0) {
                    cout << "The images are the same." << endl;
                } else {
                    cout << "The images differ by " << count << " pixels." << endl;
                }

                break;
            }

            case 5:
            {
                int angle;
                do {
                    angle = getInteger("Please enter an angle in degrees(0 - 360):  ");
                } while (angle > 360 || angle < 0);

                Rotate (img, image_grid, angle);

                break;
            }
            case 6:
            {
                int radius;
                do {
                    radius = getInteger("Enter radius greater than 0: ");
                } while (radius < 0);

                Vector<double> kernel = gaussKernelForRadius(radius);

                GaussianBlur(img, image_grid, kernel);

                break;
            }


            default:
                cout << "Enter valid value." << endl;
                break;
            }

            string imagesave = getLine("Enter filename to save image or blank to skip: ");
            if (!(imagesave == "")) {
                saveImageToFilename(img, imagesave);
            }
        }

    }
    gw.clear();
}


/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 *
 * Attempts to open the image file 'filename'.
 *
 * This function returns true when the image file was successfully
 * opened and the 'img' object now contains that image, otherwise it
 * returns false.
 */
bool openImageFromFilename(GBufferedImage& img, string filename) {
    try { img.load(filename); }
    catch (...) { return false; }
    return true;
}

/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 *
 * Attempts to save the image file to 'filename'.
 *
 * This function returns true when the image was successfully saved
 * to the file specified, otherwise it returns false.
 */
bool saveImageToFilename(const GBufferedImage &img, string filename) {
    try { img.save(filename); }
    catch (...) { return false; }
    return true;
}

/* STARTER CODE HELPER FUNCTION - DO NOT EDIT
 *
 * Waits for a mouse click in the GWindow and reports click location.
 *
 * When this function returns, row and col are set to the row and
 * column where a mouse click was detected.
 */
void getMouseClickLocation(int &row, int &col) {
    GMouseEvent me;
    do {
        me = getNextEvent(MOUSE_EVENT);
    } while (me.getEventType() != MOUSE_CLICKED);
    row = me.getY();
    col = me.getX();
}

/* HELPER FUNCTION
 *
 * This is a helper function for the Gaussian blur option.
 *
 * The function takes a radius and computes a 1-dimensional Gaussian blur kernel
 * with that radius. The 1-dimensional kernel can be applied to a
 * 2-dimensional image in two separate passes: first pass goes over
 * each row and does the horizontal convolutions, second pass goes
 * over each column and does the vertical convolutions. This is more
 * efficient than creating a 2-dimensional kernel and applying it in
 * one convolution pass.
 *
 * This code is based on the C# code posted by Stack Overflow user
 * "Cecil has a name" at this link:
 * http://stackoverflow.com/questions/1696113/how-do-i-gaussian-blur-an-image-without-using-any-in-built-gaussian-functions
 *
 */
Vector<double> gaussKernelForRadius(int radius) {
    if (radius < 1) {
        Vector<double> empty;
        return empty;
    }
    Vector<double> kernel(radius * 2 + 1);
    double magic1 = 1.0 / (2.0 * radius * radius);
    double magic2 = 1.0 / (sqrt(2.0 * PI) * radius);
    int r = -radius;
    double div = 0.0;
    for (int i = 0; i < kernel.size(); i++) {
        double x = r * r;
        kernel[i] = magic2 * exp(-x * magic1);
        r++;
        div += kernel[i];
    }
    for (int i = 0; i < kernel.size(); i++) {
        kernel[i] /= div;
    }
    return kernel;
}
