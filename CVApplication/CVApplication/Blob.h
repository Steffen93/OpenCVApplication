#ifndef BLOB_H
#define BLOB_H

class Blob {
private:
	int id;
	int x;
	int y;
public:
	int getID();
	void setID(int);
	int getX();
	int getY();
	void setX(int);
	void setY(int);
	Blob(int, int);
	static int idCounter;
};

int Blob::idCounter = 0;

Blob::Blob(int _x, int _y):x(_x), y(_y), id(Blob::idCounter++){};

void Blob::setID(int newID){
	id = newID;
}

int Blob::getID(){
	return id;
}
int Blob::getX(){
	return x;
}
int Blob::getY(){
	return y;
}
void Blob::setX(int newX){
	x = newX;
}
void Blob::setY(int newY){
	y = newY;
}

#endif /*BLOB_H*/