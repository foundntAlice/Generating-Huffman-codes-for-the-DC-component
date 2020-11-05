#include<opencv2/core.hpp>
#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>

#include<iostream>
#include<math.h>
#include<algorithm>
#include<map>

using namespace std;
using namespace cv;
#define PI 3.1415926
#define SIZE  512 //the size of the lena image is 512x512
#define MAX_TREE_HT 100

float R[SIZE][SIZE], G[SIZE][SIZE], B[SIZE][SIZE];
//quantization table for luminance
int qtzY[8][8] = { {16, 11, 10, 16, 24, 40, 51, 61},
				  {12, 12, 14, 19, 26, 58, 60, 55},
				  {14, 13, 16, 24, 40, 57, 69, 56},
				  {14, 17, 22, 29, 51, 87, 80, 62},
				  {18, 22, 37, 56, 68, 109, 103, 77},
				  {24, 35, 55, 64, 81, 104, 113, 92},
				  {49, 64, 78, 87, 103, 121, 120, 101},
				  {72, 92, 95, 98, 112, 100, 103, 99} };

//quantization table for chrominance
int qtzC[8][8] = { {17, 18, 24,	47,	99,	99,	99,	99},
	               {18, 21, 26, 66, 99, 99, 99, 99},
	               {24,	26,	56,	99,	99,	99,	99,	99},
             	   {47,	66,	99,	99,	99,	99,	99,	99},
	               {99,	99,	99,	99,	99,	99,	99,	99},
	               {99,	99,	99,	99,	99,	99,	99,	99},
	               {99,	99,	99,	99,	99,	99,	99,	99},
	               {99,	99,	99,	99,	99,	99,	99,	99}};

void DCT_Transform(int u, int v, vector<vector<int>> &out, vector<vector<int>> &in) {
	for (int i = u; i < u + 8; i++) {
		for (int j = v; j < v + 8; j++) {
			double tmp = 0;
			double Cu = 1, Cv = 1;
			if (i == u) Cu = 1 / sqrt(2);
			if (j == v) Cv = 1 / sqrt(2);
			for (int x = 0; x < 8; x++) {
				for (int y = 0; y < 8; y++) {
					tmp += 0.25 * Cu * Cv * in[u + x][v + y] * cos((2 * x + 1) * (i - u) * PI / 16) * cos((2 * y + 1) * (j - v) * PI / 16);
				}
			}
			out[i][j] = (int)tmp;
		}
	}
} 
void quantization(int u, int v, vector<vector<int>>& out, vector<vector<int>> dct, int qtz[][8]) {
	for (int i = u; i < u + 8; i++) {
		for (int j = v; j < v + 8; j++) {
			out[i][j] = (int)((double)dct[i][j] / qtz[i - u][j - v] + 0.5);
		}
	}
}
//zig-zag scan algorithm
vector<int> zigZagScan(vector<vector<int>> &v){
	int n = 64, m = 64;
	vector<int> res(4096);
	int index = 0;
	int row = 0, col = 0;
	bool row_inc = 0;
	int mn = min(m, n);
	for (int len = 1; len <= mn; ++len) {
		for (int i = 0; i < len; ++i) {
			res[index++] = v[row][col];
			if (i + 1 == len)
				break;
			if (row_inc)
				++row, --col;
			else
				--row, ++col;
		}
		if (len == mn)
			break;
		if (row_inc)
			++row, row_inc = false;
		else
			++col, row_inc = true;
	}
	if (row == 0) {
		if (col == m - 1)
			++row;
		else
			++col;
		row_inc = 1;
	}
	else {
		if (row == n - 1)
			++col;
		else
			++row;
		row_inc = 0;
	}
	int MAX = max(m, n) - 1;
	for (int len, diag = MAX; diag > 0; --diag) {

		if (diag > mn)
			len = mn;
		else
			len = diag;

		for (int i = 0; i < len; ++i) {
			res[index++] = v[row][col];
			if (i + 1 == len)
				break;
			if (row_inc)
				++row, --col;
			else
				++col, --row;
		}
		if (row == 0 || col == m - 1) {
			if (col == m - 1)
				++row;
			else
				++col;

			row_inc = true;
		}
		else if (col == 0 || row == n - 1) {
			if (row == n - 1)
				++col;
			else
				++row;
			row_inc = false;
		}
	}
	return res;
}
//generating Huffman code 
struct MinHeapNode {
	int data;
	unsigned freq;
	struct MinHeapNode* left, * right;
};
struct MinHeap {
	unsigned size;
	unsigned capacity;
	struct MinHeapNode** array;
};
struct MinHeapNode* newNode(int data, unsigned freq){
	struct MinHeapNode* temp = (struct MinHeapNode*)malloc(sizeof(struct MinHeapNode));
	temp->left = temp->right = NULL;
	temp->data = data;
	temp->freq = freq;
	return temp;
}
struct MinHeap* createMinHeap(unsigned capacity){
	struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
	minHeap->size = 0;
	minHeap->capacity = capacity;
	minHeap->array = (struct MinHeapNode**)malloc(minHeap->capacity * sizeof(struct MinHeapNode*));
	return minHeap;
}
void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b){
	struct MinHeapNode* t = *a;
	*a = *b;
	*b = t;
} 
void minHeapify(struct MinHeap* minHeap, int idx){
	int smallest = idx;
	int left = 2 * idx + 1;
	int right = 2 * idx + 2;

	if (left < minHeap->size && minHeap->array[left]->
		freq < minHeap->array[smallest]->freq)
		smallest = left;

	if (right < minHeap->size && minHeap->array[right]->
		freq < minHeap->array[smallest]->freq)
		smallest = right;

	if (smallest != idx) {
		swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
		minHeapify(minHeap, smallest);
	}
}
int isSizeOne(struct MinHeap* minHeap){
	return (minHeap->size == 1);
}
struct MinHeapNode* extractMin(struct MinHeap* minHeap){
	struct MinHeapNode* temp = minHeap->array[0];
	minHeap->array[0] = minHeap->array[minHeap->size - 1];
	--minHeap->size;
	minHeapify(minHeap, 0);
	return temp;
}
void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* minHeapNode){
	++minHeap->size;
	int i = minHeap->size - 1;
	while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) {
		minHeap->array[i] = minHeap->array[(i - 1) / 2];
		i = (i - 1) / 2;
	}
	minHeap->array[i] = minHeapNode;
}
void buildMinHeap(struct MinHeap* minHeap){
	int n = minHeap->size - 1;
	int i;
	for (i = (n - 1) / 2; i >= 0; --i)
		minHeapify(minHeap, i);
}
string binaryCode(int arr[], int n){
	string res = "";
	int i;
	for (i = 0; i < n; ++i) res += arr[i] + '0';
	return res;
}
int isLeaf(struct MinHeapNode* root){
	return !(root->left) && !(root->right);
}
struct MinHeap* createAndBuildMinHeap(vector<int> &data, vector<int> &freq, int size){
	struct MinHeap* minHeap = createMinHeap(size);
	for (int i = 0; i < size; ++i)
		minHeap->array[i] = newNode(data[i], freq[i]);
	minHeap->size = size;
	buildMinHeap(minHeap);
	return minHeap;
}
struct MinHeapNode* buildHuffmanTree(vector<int> &data, vector<int> &freq, int size){
	struct MinHeapNode* left, * right, * top;
	struct MinHeap* minHeap = createAndBuildMinHeap(data, freq, size);
	while (!isSizeOne(minHeap)) {
		left = extractMin(minHeap);
		right = extractMin(minHeap);
		top = newNode(0, left->freq + right->freq);
		top->left = left;
		top->right = right;
		insertMinHeap(minHeap, top);
	}
	return extractMin(minHeap);
}
void getCodes(struct MinHeapNode* root, int arr[], int top, vector<pair<int, string>> &res){
	if (root->left) {
		arr[top] = 0;
		getCodes(root->left, arr, top + 1, res);
	}
	if (root->right) {
		arr[top] = 1;
		getCodes(root->right, arr, top + 1, res);
	}
	if (isLeaf(root)) {
		int pattern = root->data;
		string code = binaryCode(arr, top);
		res.push_back({ pattern, code });
	}
}
vector<pair<int, string>> HuffmanCodes(vector<int> &data, vector<int> &freq, int size){
	vector<pair<int, string>> res;
	struct MinHeapNode* root = buildHuffmanTree(data, freq, size);
	int arr[MAX_TREE_HT], top = 0;
	getCodes(root, arr, top, res);
	return res;
}
// main function start from here
int main() {
	ios_base::sync_with_stdio(0); cout.tie(0);
	Mat lena = imread("C:/Users/falli/OneDrive/Desktop/draft/lenargb.jpg", IMREAD_COLOR);
	Mat RGB[3];
	Mat blue, green, red;
	split(lena, RGB); //split lena image to RBG channels with BGR order

	RGB[0].convertTo(blue, CV_32F);
	RGB[1].convertTo(green, CV_32F);
	RGB[2].convertTo(red, CV_32F);
	
	//create 3 2D arrays to store R G B values of the lena image

	for (int i = 0; i < SIZE; i++)
	{
		const float* blueRow = blue.ptr<float>(i);
		const float* greenRow = green.ptr<float>(i);
		const float* redRow = red.ptr<float>(i);
		for (int j = 0; j < SIZE; j++)
		{
			B[i][j] = blueRow[j];
			G[i][j] = greenRow[j];
			R[i][j] = redRow[j];
		}
	}
	//transform image to luminance/chrominance space
	vector<vector<int>> Y(SIZE, vector<int>(SIZE));
	vector<vector<int>> Cb(SIZE, vector<int>(SIZE));
	vector<vector<int>> Cr(SIZE, vector<int>(SIZE));
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			Y[i][j] = (int)(0.29900 * R[i][j] + 0.58700 * G[i][j] + 0.11400 * B[i][j]);
			Cb[i][j] = (int)(-0.16874 * R[i][j] - 0.33126 * G[i][j] + 0.5000 * B[i][j]);
			Cr[i][j] = (int)(0.50000 * R[i][j] - 0.41869 * G[i][j] - 0.08131 * B[i][j]);
		}
	}
	//subtract 128 from each pixel value
	for (int i = 0; i < 512; i++) {
		for (int j = 0; j < 512; j++) {
			Y[i][j] -= 128;
			Cb[i][j] -= 128;
			Cr[i][j] -= 128;
		}
	}
	//perform DCT on each block 8x8
	vector<vector<int>> DCT_Y(SIZE, vector<int>(SIZE));
	vector<vector<int>> DCT_Cb(SIZE, vector<int>(SIZE));
	vector<vector<int>> DCT_Cr(SIZE, vector<int>(SIZE));
	for (int i = 0; i < SIZE; i += 8){
		for (int j = i; j < SIZE; j += 8) {
			DCT_Transform(i, j, DCT_Y, Y);
			DCT_Transform(i, j, DCT_Cb, Cb);
			DCT_Transform(i, j, DCT_Cr, Cr);
		}
	}
	//perform quantization for DCT_Y, DCT_Cb and DCT_Cr
	vector<vector<int>> quantizedY(SIZE, vector<int>(SIZE));
	vector<vector<int>> quantizedCb(SIZE, vector<int>(SIZE));
	vector<vector<int>> quantizedCr(SIZE, vector<int>(SIZE));
	for (int i = 0; i < SIZE; i += 8) {
		for (int j = i; j < SIZE; j += 8) {
			quantization(i, j, quantizedY, DCT_Y, qtzY);
			quantization(i, j, quantizedCb, DCT_Cb, qtzC);
			quantization(i, j, quantizedCr, DCT_Cr, qtzC);
		}
	}
	//applying Differential Pulse Code Modulation to the DC component
	//storing DC values into 64x64 vector
	vector<vector<int>> zigZagY(64, vector<int>(64));
	vector<vector<int>> zigZagCb(64, vector<int>(64));
	vector<vector<int>> zigZagCr(64, vector<int>(64));
	for (int i = 0; i < SIZE; i += 8) {
		for (int j = i; j < SIZE; j += 8) {
			zigZagY[i / 8][j / 8] = quantizedY[i][j];
			zigZagCb[i / 8][j / 8] = quantizedCb[i][j];
			zigZagCr[i / 8][j / 8] = quantizedCr[i][j];
		}
	}
	vector<int> dpcmY(4096), dpcmCb(4096), dpcmCr(4096);
	vector<int> zzY = zigZagScan(zigZagY);
	vector<int> zzCb = zigZagScan(zigZagCb);
	vector<int> zzCr = zigZagScan(zigZagCr);
	for (int i = 0; i < 4096; i++) {
		if (i == 0) {
			dpcmY[i] = zzY[i];
			dpcmCb[i] = zzCb[i];
			dpcmCr[i] = zzCr[i];
		}
		else {
			dpcmY[i] = zzY[i] - zzY[i - 1];
			dpcmCb[i] = zzCb[i] - zzCb[i - 1];
			dpcmCr[i] = zzCr[i] - zzCr[i - 1];
		}
	}
	//creating 2 vectors for each Y/CbCr to get the frequency of each DPCM element
	vector<int> elementsY, frequencyY;
	vector<int> elementsC, frequencyC;
	map<int, int> countt;
	for (auto x : dpcmY) ++countt[x];
	for (auto x : countt) {
		elementsY.push_back(x.first);
		frequencyY.push_back(x.second);
	}
	countt.clear();
	for (auto x : dpcmCb) ++countt[x];
	for (auto x : dpcmCr) ++countt[x];
	for (auto x : countt) {
		elementsC.push_back(x.first);
		frequencyC.push_back(x.second);
	}
	//generating Huffman code tables for Y and CbCr
	vector<pair<int, string>> luminanceCode = HuffmanCodes(elementsY, frequencyY, elementsY.size());
	sort(luminanceCode.begin(), luminanceCode.end());
	vector<pair<int, string>> chrominanceCode = HuffmanCodes(elementsC, frequencyC, elementsC.size());
	sort(chrominanceCode.begin(), chrominanceCode.end());
	cout << "The codes for the luminance of the DC component are:\n";
	for (auto x : luminanceCode) {
		cout << x.first << "  " << x.second << endl;
	}
	cout << "\nThe codes for the chrominance of the DC component are:\n";
	for (auto x : chrominanceCode) {
		cout << x.first << "  " << x.second << endl;
	}

	return 0; 
} 
