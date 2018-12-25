#ifndef __UTILITIES__
#define __UTILITIES__

#define SIZEofBUFF 20
#define SSizeofBUFF 6

typedef struct record{
	long  custid;
	char 	FirstName[SIZEofBUFF];
	char 	LastName[SIZEofBUFF];
	char	Street[SIZEofBUFF];
	int 	HouseID;
	char	City[SIZEofBUFF];
	char	postcode[SSizeofBUFF];
	float  amount;
} record_t;


#endif
