// Fill out your copyright notice in the Description page of Project Settings.

#include "Canyons.h"
#include "Maze.h"
#include <algorithm>
#include <time.h>
#include <array>

struct Entrance
{
	uint16 OriginRow, OriginCol, SetID1, SetID2, NewPath;

	bool operator==(const Entrance &other) const
	{
		if (SetID1 == other.SetID1 && SetID2 == other.SetID2)
		{
			return true;
		}
		else if (SetID1 == other.SetID2 && SetID2 == other.SetID1)
		{
			return true;
		}
		return false;
	}
};

// Sets default values
AMaze::AMaze(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer), Height(31), Width(31), BossSize(9), NextID(1)
{
	static ConstructorHelpers::FObjectFinder<UBlueprint> PutNameHere(TEXT("/Game/Blueprints/TileBP.TileBP"));
	if (PutNameHere.Object) {
		BlueprintVar = (UClass*)PutNameHere.Object->GeneratedClass;
	}
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Initialize();
}

//AMaze::AMaze(const FObjectInitializer &ObjectInitializer, uint16 High, uint16 Wide) : Super(ObjectInitializer), Height(High), Width(Wide), BossSize(9)
//{
//	static ConstructorHelpers::FObjectFinder<UBlueprint> PutNameHere(TEXT("/Game/Blueprints/TileBP.TileBP"));
//	if (PutNameHere.Object) {
//		BlueprintVar = (UClass*)PutNameHere.Object->GeneratedClass;
//	}
//
//	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
//	PrimaryActorTick.bCanEverTick = false;
//
//	Height |= 1;
//	Width |= 1;
//
//	Initialize();
//}

void AMaze::Initialize()
{
	srand(time(NULL));

	MyMaze.Row.SetNum(Height);
	for (uint16 i = 0; i < Height; ++i)
	{
		MyMaze.Row[i].Col.SetNumZeroed(Width);
	}


	PlaceRoom(1, 1, 9, 9);
	GenerateRooms(40);
	for (int Row = 1; Row < Height; Row += 2)
	{
		for (int Col = 1; Col < Width; Col += 2)
		{
			DepthFirstMaze(Row, Col);
		}
	}
	GenerateEntrances();
	RemoveAllDeadEnds();
	SpawnTiles();
}

// Called when the game starts or when spawned
void AMaze::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AMaze::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMaze::GenerateRooms(uint16 Tries)//Places rooms of random size in the maze, makes Tries attempts
{
	uint16 Wide, High, OriginRow, OriginCol;
	uint16 MaxSize = BossSize - 3;
	for (uint16 i = 0; i < Tries; ++i){
		Wide = (rand() % MaxSize) + 2;
		High = (rand() % MaxSize) + 2;
		//Add 1 to random number if it is even. Implemented with bitwise OR to 1
		Wide |= 1;
		High |= 1;
		OriginRow = rand() % (Height - High - 1);
		OriginCol = rand() % (Width - Wide - 1);
		OriginRow |= 1;
		OriginCol |= 1;
		PlaceRoom(OriginRow, OriginCol, High, Wide);
	}
}

void AMaze::PlaceRoom(uint16 OriginRow, uint16 OriginCol, uint16 High, uint16 Wide)
{
	//Rooms must have odd dimensions and locations, otherwise return
	if (OriginRow % 2 == 0 || OriginCol % 2 == 0 || High % 2 == 0 || Wide % 2 == 0)
	{
		return;
	}
	//check if room space is already filled
	for (uint16 i = OriginRow; i < High + OriginRow; i = i + 2)
	{
		for (uint16 j = OriginCol; j < Wide + OriginCol; j = j + 2)
		{
			if (MyMaze.Row[i].Col[j] == true)
			{
				//If any of the spaces are filled, the room cannot be made and the function exits
				return;
			}
		}
	}

	for (uint16 i = OriginRow; i < High + OriginRow; ++i)
	{
		for (uint16 j = OriginCol; j < Wide + OriginCol; ++j)
		{
			MyMaze.Row[i].Col[j] = true;
		}
	}
	FRoom Temp;
	Temp.OriginCol = OriginCol;
	Temp.OriginRow = OriginRow;
	Temp.High = High;
	Temp.Wide = Wide;
	Temp.SetID = NextID;
	++NextID;

	Rooms.Add(Temp);
}

void AMaze::GenerateMaze()
{

}

//Generates a maze using Depth-first algorith starting at origin(x,y)
void AMaze::DepthFirstMaze(uint16 OriginRow, uint16 OriginCol)
{
	//Maze must start at a place with an odd location
	if (OriginRow % 2 == 0 || OriginCol % 2 == 0)
	{
		return;
	}

	uint16 PathIndex = 0;

	if (MyMaze.Row[OriginRow].Col[OriginCol] == 0)
	{
		//create and record index of new path
		Paths.AddDefaulted(1);
		PathIndex = Paths.Num() - 1;
		Paths[PathIndex].SetID = NextID;
		++NextID;

		MyMaze.Row[OriginRow].Col[OriginCol] = 1;
		FWalkway Temp;
		Temp.Col = OriginCol;
		Temp.Row = OriginRow;
		Paths[PathIndex].Tiles.Add(Temp);
	}
	else
	{
		return;
	}

	DepthFirstMazeHelper(OriginRow, OriginCol, PathIndex);
}

void AMaze::DepthFirstMazeHelper(uint16 OriginRow, uint16 OriginCol, uint16 PathIndex)
{
	MyMaze.Row[OriginRow].Col[OriginCol] = 1;
	FWalkway Temp;
	Temp.Col = OriginCol;
	Temp.Row = OriginRow;
	Paths[PathIndex].Tiles.Add(Temp);

	uint8 Directions[4] = { 1, 2, 3, 4 };
	std::random_shuffle(std::begin(Directions), std::end(Directions));
	for (int8 i = 0; i < 4; i++)
	{
		//TODO: IMPLEMENT USING NEW FUNCTION
		switch (Directions[i])
		{
		case 1: //Right
			if (OriginRow > 0 && OriginRow < Height && OriginCol > 0 && OriginCol + 2 < Width)
			{
				if (MyMaze.Row[OriginRow].Col[OriginCol + 2] == 0)
				{
					MyMaze.Row[OriginRow].Col[OriginCol + 1] = 1;
					FWalkway Temp;
					Temp.Col = OriginCol + 1;
					Temp.Row = OriginRow;
					Paths[PathIndex].Tiles.Add(Temp);

					DepthFirstMazeHelper(OriginRow, OriginCol + 2, PathIndex);
				}
			}
			break;
		case 2: //Down
			if (OriginRow > 0 && OriginRow + 2 < Height && OriginCol > 0 && OriginCol < Width)
			{
				if (MyMaze.Row[OriginRow + 2].Col[OriginCol] == 0)
				{
					MyMaze.Row[OriginRow + 1].Col[OriginCol] = 1;
					FWalkway Temp;
					Temp.Col = OriginCol;
					Temp.Row = OriginRow + 1;
					Paths[PathIndex].Tiles.Add(Temp);

					DepthFirstMazeHelper(OriginRow + 2, OriginCol, PathIndex);
				}
			}
			break;
		case 3: //Left
			if (OriginRow > 0 && OriginRow < Height && OriginCol - 2 > 0 && OriginCol < Width)
			{
				if (MyMaze.Row[OriginRow].Col[OriginCol - 2] == 0)
				{
					MyMaze.Row[OriginRow].Col[OriginCol - 1] = 1;
					FWalkway Temp;
					Temp.Col = OriginCol - 1;
					Temp.Row = OriginRow;
					Paths[PathIndex].Tiles.Add(Temp);

					DepthFirstMazeHelper(OriginRow, OriginCol - 2, PathIndex);
				}
			}
			break;
		case 4: //Up
			if (OriginRow - 2 > 0 && OriginRow < Height && OriginCol > 0 && OriginCol < Width)
			{
				if (MyMaze.Row[OriginRow - 2].Col[OriginCol] == 0)
				{
					MyMaze.Row[OriginRow - 1].Col[OriginCol] = 1;
					FWalkway Temp;
					Temp.Col = OriginCol;
					Temp.Row = OriginRow - 1;
					Paths[PathIndex].Tiles.Add(Temp);

					DepthFirstMazeHelper(OriginRow - 2, OriginCol, PathIndex);
				}
			}
			break;
		default:
			;//does not execute, all values are 1-4
		}
	}
}

//Generates a maze using Primm algorith starting at origin(x,y)
void AMaze::PrimmMaze(uint16 OriginRow, uint16 OriginCol)
{
	//Maze must start at a place with an odd location
	if (OriginRow % 2 == 0 || OriginCol % 2 == 0)
	{
		return;
	}

	//Array of 4 values, two pair of coordinates, one representing the tile to be traveled to and one representing the tile traveled across 
	TArray<std::pair<std::pair<uint16, uint16>, std::pair<uint16, uint16>>> Coordinates;
	//number to keep track of the index of a new path, if created
	uint16 PathIndex = 0;

	if (MyMaze.Row[OriginRow].Col[OriginCol] == 0)
	{
		//create and record index of new path
		Paths.AddDefaulted(1);
		PathIndex = Paths.Num() - 1;
		Paths[PathIndex].SetID = NextID;
		++NextID;

		//Add starting point to new path
		MyMaze.Row[OriginRow].Col[OriginCol] = 1;
		FWalkway Temp;
		Temp.Col = OriginCol;
		Temp.Row = OriginRow;
		Paths[PathIndex].Tiles.Add(Temp);
	}
	else
	{
		return;
	}

	//Add Up, Down, Left, Right to list
	if (CheckAvailable(OriginRow + 2, OriginCol))
	{
		Coordinates.Add(std::make_pair(std::make_pair(OriginRow + 2, OriginCol), std::make_pair(OriginRow + 1, OriginCol)));
	}
	if (CheckAvailable(OriginRow - 2, OriginCol))
	{
		Coordinates.Add(std::make_pair(std::make_pair(OriginRow - 2, OriginCol), std::make_pair(OriginRow - 1, OriginCol)));
	}
	if (CheckAvailable(OriginRow, OriginCol + 2))
	{
		Coordinates.Add(std::make_pair(std::make_pair(OriginRow, OriginCol + 2), std::make_pair(OriginRow, OriginCol + 1)));
	}
	if (CheckAvailable(OriginRow, OriginCol - 2))
	{
		Coordinates.Add(std::make_pair(std::make_pair(OriginRow, OriginCol - 2), std::make_pair(OriginRow, OriginCol - 1)));
	}

	//Loop through list, randomly digging a tunnel, then adding its 
	while (Coordinates.Num() != 0)
	{
		int Cur = rand() % Coordinates.Num();
		int OddRow = Coordinates[Cur].first.first;
		int OddCol = Coordinates[Cur].first.second;
		int WallRow = Coordinates[Cur].second.first;
		int WallCol = Coordinates[Cur].second.second;
		if (CheckAvailable(OddRow, OddCol))
		{
			MyMaze.Row[OddRow].Col[OddCol] = 1;
			FWalkway Temp;
			Temp.Col = OddCol;
			Temp.Row = OddRow;
			Paths[PathIndex].Tiles.Add(Temp);

			MyMaze.Row[WallRow].Col[WallCol] = 1;
			FWalkway Temp2;
			Temp2.Col = WallCol;
			Temp2.Row = WallRow;
			Paths[PathIndex].Tiles.Add(Temp2);

			if (CheckAvailable(OddRow + 2, OddCol))
			{
				Coordinates.Add(std::make_pair(std::make_pair(OddRow + 2, OddCol), std::make_pair(OddRow + 1, OddCol)));
			}
			if (CheckAvailable(OddRow - 2, OddCol))
			{
				Coordinates.Add(std::make_pair(std::make_pair(OddRow - 2, OddCol), std::make_pair(OddRow - 1, OddCol)));
			}
			if (CheckAvailable(OddRow, OddCol + 2))
			{
				Coordinates.Add(std::make_pair(std::make_pair(OddRow, OddCol + 2), std::make_pair(OddRow, OddCol + 1)));
			}
			if (CheckAvailable(OddRow, OddCol - 2))
			{
				Coordinates.Add(std::make_pair(std::make_pair(OddRow, OddCol - 2), std::make_pair(OddRow, OddCol - 1)));
			}
		}
		Coordinates.RemoveAt(Cur);
	}
}

void AMaze::GenerateEntrances()
{
	TArray<Entrance> PotentialEntrances;
	//For each room add all spaces that connect that room to another tile to Potential Entrances
	for (int CurRoom = 0; CurRoom < Rooms.Num(); ++CurRoom)
	{
		//Check that the room is not on the border, preventing an entrance on that side
		if (Rooms[CurRoom].OriginCol > 1)
		{
			//Iterate along the side, along all odd spaces
			for (int i = 0; i < Rooms[CurRoom].High; i += 2)
			{
				//if the space across the way is filled, it is a valid entrance
				if (MyMaze.Row[Rooms[CurRoom].OriginRow + i].Col[Rooms[CurRoom].OriginCol - 2] == 1)
				{
					bool bIsRoom1 = false, bIsRoom2 = false;
					Entrance Temp;
					//set the entrance's location
					Temp.OriginCol = Rooms[CurRoom].OriginCol - 1;
					Temp.OriginRow = Rooms[CurRoom].OriginRow + i;
					//decide which sets the entrances connect to, and if it connects two rooms together
					Temp.SetID1 = FindTileOwner(Rooms[CurRoom].OriginRow + i, Rooms[CurRoom].OriginCol, bIsRoom1);
					Temp.SetID2 = FindTileOwner(Rooms[CurRoom].OriginRow + i, Rooms[CurRoom].OriginCol - 2, bIsRoom2);
					
					//if you connect two rooms it will be a new path you are creating (represented by 0), otherwise you save which path you are adding to
					if (bIsRoom1 == true && bIsRoom2 == true)
					{
						Temp.NewPath = 0;
					}
					else
					{
						if (bIsRoom1)
						{
							Temp.NewPath = Temp.SetID2;
						}
						else
						{
							Temp.NewPath = Temp.SetID1;
						}
					}

					PotentialEntrances.Add(Temp);
				}
			}
		}
		if (Rooms[CurRoom].OriginRow > 1)
		{
			for (int i = 0; i < Rooms[CurRoom].Wide; i += 2)
			{
				if (MyMaze.Row[Rooms[CurRoom].OriginRow - 2].Col[Rooms[CurRoom].OriginCol + i] == 1)
				{
					bool bIsRoom1 = false, bIsRoom2 = false;
					Entrance Temp;
					Temp.OriginCol = Rooms[CurRoom].OriginCol + i;
					Temp.OriginRow = Rooms[CurRoom].OriginRow - 1;
					Temp.SetID1 = FindTileOwner(Rooms[CurRoom].OriginRow, Rooms[CurRoom].OriginCol + i, bIsRoom1);
					Temp.SetID2 = FindTileOwner(Rooms[CurRoom].OriginRow - 2, Rooms[CurRoom].OriginCol + i, bIsRoom2);
					
					if (bIsRoom1 == true && bIsRoom2 == true)
					{
						Temp.NewPath = 0;
					}
					else
					{
						if (bIsRoom1)
						{
							Temp.NewPath = Temp.SetID2;
						}
						else
						{
							Temp.NewPath = Temp.SetID1;
						}
					}

					PotentialEntrances.Add(Temp);
				}
			}
		}
		if (Rooms[CurRoom].OriginCol + Rooms[CurRoom].Wide < Width - 3)
		{
			for (int i = 0; i < Rooms[CurRoom].High; i += 2)
			{
				if (MyMaze.Row[Rooms[CurRoom].OriginRow + i].Col[Rooms[CurRoom].OriginCol + Rooms[CurRoom].Wide + 1] == 1)
				{
					bool bIsRoom1 = false, bIsRoom2 = false;
					Entrance Temp;
					Temp.OriginCol = Rooms[CurRoom].OriginCol + Rooms[CurRoom].Wide;
					Temp.OriginRow = Rooms[CurRoom].OriginRow + i;
					Temp.SetID1 = FindTileOwner(Rooms[CurRoom].OriginRow + i, Rooms[CurRoom].OriginCol + Rooms[CurRoom].Wide - 1, bIsRoom1);
					Temp.SetID2 = FindTileOwner(Rooms[CurRoom].OriginRow + i, Rooms[CurRoom].OriginCol + Rooms[CurRoom].Wide + 1, bIsRoom2);

					if (bIsRoom1 == true && bIsRoom2 == true)
					{
						Temp.NewPath = 0;
					}
					else
					{
						if (bIsRoom1)
						{
							Temp.NewPath = Temp.SetID2;
						}
						else
						{
							Temp.NewPath = Temp.SetID1;
						}
					}

					PotentialEntrances.Add(Temp);
				}
			}
		}
		if (Rooms[CurRoom].OriginRow + Rooms[CurRoom].High < Height - 3)
		{
			for (int i = 0; i < Rooms[CurRoom].Wide; i += 2)
			{
				if (MyMaze.Row[Rooms[CurRoom].OriginRow + Rooms[CurRoom].High + 1].Col[Rooms[CurRoom].OriginCol + i] == 1)
				{
					bool bIsRoom1 = false, bIsRoom2 = false;
					Entrance Temp;
					Temp.OriginCol = Rooms[CurRoom].OriginCol + i;
					Temp.OriginRow = Rooms[CurRoom].OriginRow + Rooms[CurRoom].High;
					Temp.SetID1 = FindTileOwner(Rooms[CurRoom].OriginRow + Rooms[CurRoom].High + 1, Rooms[CurRoom].OriginCol + i, bIsRoom1);
					Temp.SetID2 = FindTileOwner(Rooms[CurRoom].OriginRow + Rooms[CurRoom].High - 1, Rooms[CurRoom].OriginCol + i, bIsRoom2);

					if (bIsRoom1 == true && bIsRoom2 == true)
					{
						Temp.NewPath = 0;
					}
					else
					{
						if (bIsRoom1)
						{
							Temp.NewPath = Temp.SetID2;
						}
						else
						{
							Temp.NewPath = Temp.SetID1;
						}
					}

					PotentialEntrances.Add(Temp);
				}
			}
		}
	}
	while (PotentialEntrances.Num() > 0)
	{
		uint16 i = rand() % PotentialEntrances.Num();
		MyMaze.Row[PotentialEntrances[i].OriginRow].Col[PotentialEntrances[i].OriginCol] = 1;
		
		uint16 PathIndex;
		if (PotentialEntrances[i].NewPath == 0)
		{
			//create and record index of new path
			Paths.AddDefaulted(1);
			PathIndex = Paths.Num() - 1;
			Paths[PathIndex].SetID = NextID;
			++NextID;

			//Add point to new path
			FWalkway Temp;
			Temp.Col = PotentialEntrances[i].OriginCol;
			Temp.Row = PotentialEntrances[i].OriginRow;
			Paths[PathIndex].Tiles.Add(Temp);
		}
		else
		{
			//get the index of the path with ID NewPath
			for (PathIndex = 0; PathIndex < Paths.Num(); ++PathIndex)
			{
				if (Paths[PathIndex].SetID == PotentialEntrances[i].NewPath)
				{
					break;
				}
			}

			
			//Add point to path
			FWalkway Temp;
			Temp.Col = PotentialEntrances[i].OriginCol;
			Temp.Row = PotentialEntrances[i].OriginRow;

			Paths[PathIndex].Tiles.Add(Temp);
		}
		Entrance Temp = PotentialEntrances[i];
		PotentialEntrances.RemoveSwap(Temp);
	}
}

uint16 AMaze::FindTileOwner(uint16 OriginRow, uint16 OriginCol, bool &OutIsRoom)
{
	for (int i = 0; i < Rooms.Num(); ++i)
	{
		if (Rooms[i].Contains(OriginRow, OriginCol))
		{
			OutIsRoom = true;
			return Rooms[i].SetID;
		}
	}
	for (int i = 0; i < Paths.Num(); ++i)
	{
		FWalkway Temp;
		Temp.Col = OriginCol;
		Temp.Row = OriginRow;
		if (Paths[i].Tiles.Contains(Temp))
		{
			OutIsRoom = false;
			return Paths[i].SetID;
		}
	}
	return 0;
}

void AMaze::UpdateBossSize(uint8 NewSize)
{
	//Boss room must have an odd dimension
	if (NewSize % 2 == 0)
	{
		return;
	}
	BossSize = NewSize;
}

void AMaze::RemoveAllDeadEnds()
{
	for (uint16 Row = 1; Row < Height; Row += 2)
	{
		for (uint16 Col = 1; Col < Width; Col += 2)
		{
			RemoveAllDeadEndsHelper(Row, Col);
		}
	}
}

void AMaze::RemoveAllDeadEndsHelper(uint16 OriginRow, uint16 OriginCol)
{
	uint8 Sum = MyMaze.Row[OriginRow + 1].Col[OriginCol] + MyMaze.Row[OriginRow].Col[OriginCol + 1] + MyMaze.Row[OriginRow - 1].Col[OriginCol] + MyMaze.Row[OriginRow].Col[OriginCol - 1];
	//If it is a dead end
	if (Sum == 1 || Sum == 0)
	{
		//Remove current tile
		MyMaze.Row[OriginRow].Col[OriginCol] = 0;
		FWalkway Temp;
		Temp.Col = OriginCol;
		Temp.Row = OriginRow;
		for (uint16 i = 0; i < Paths.Num(); ++i)
		{
			Paths[i].Tiles.Remove(Temp);
			if (Paths[i].Tiles.Num() == 0)
			{
				Paths.RemoveAt(i);
			}
		}
		
		//Remove the tile from which we were a dead end
		if (MyMaze.Row[OriginRow + 1].Col[OriginCol] == 1)
		{
			MyMaze.Row[OriginRow + 1].Col[OriginCol] = 0;
			FWalkway Temp;
			Temp.Col = OriginCol;
			Temp.Row = OriginRow + 1;
			for (uint16 i = 0; i < Paths.Num(); ++i)
			{
				Paths[i].Tiles.Remove(Temp);
				if (Paths[i].Tiles.Num() == 0)
				{
					Paths.RemoveAt(i);
				}
			}
			RemoveAllDeadEndsHelper(OriginRow + 2, OriginCol);
		}
		else if (MyMaze.Row[OriginRow].Col[OriginCol + 1] == 1)
		{
			MyMaze.Row[OriginRow].Col[OriginCol + 1] = 0;
			FWalkway Temp;
			Temp.Col = OriginCol + 1;
			Temp.Row = OriginRow;
			for (uint16 i = 0; i < Paths.Num(); ++i)
			{
				Paths[i].Tiles.Remove(Temp);
				if (Paths[i].Tiles.Num() == 0)
				{
					Paths.RemoveAt(i);
				}
			}
			RemoveAllDeadEndsHelper(OriginRow, OriginCol + 2);
		}
		else if (MyMaze.Row[OriginRow - 1].Col[OriginCol] == 1)
		{
			MyMaze.Row[OriginRow - 1].Col[OriginCol] = 0;
			FWalkway Temp;
			Temp.Col = OriginCol;
			Temp.Row = OriginRow - 1;
			for (uint16 i = 0; i < Paths.Num(); ++i)
			{
				Paths[i].Tiles.Remove(Temp);
				if (Paths[i].Tiles.Num() == 0)
				{
					Paths.RemoveAt(i);
				}
			}
			RemoveAllDeadEndsHelper(OriginRow - 2, OriginCol);
		}
		else if (MyMaze.Row[OriginRow].Col[OriginCol - 1] == 1)
		{
			MyMaze.Row[OriginRow].Col[OriginCol - 1] = 0;
			FWalkway Temp;
			Temp.Col = OriginCol - 1;
			Temp.Row = OriginRow;
			for (uint16 i = 0; i < Paths.Num(); ++i)
			{
				Paths[i].Tiles.Remove(Temp);
				if (Paths[i].Tiles.Num() == 0)
				{
					Paths.RemoveAt(i);
				}
			}
			RemoveAllDeadEndsHelper(OriginRow, OriginCol - 2);
		}
	
	}
}

void AMaze::SpawnTiles()
{
	for (uint16 r = 0; r < Height; r++)
	{
		for (uint16 c = 0; c < Width; c++)
		{
			if (MyMaze.Row[r].Col[c])
			{
				SpawnSingleTile(r, c);
			}
		}
	}
}

void AMaze::SpawnPaths()
{
	for (uint16 CurPath = 0; CurPath < Paths.Num(); ++CurPath)
	{
		for (uint16 CurTile = 0; CurTile < Paths[CurPath].Tiles.Num(); ++CurTile)
		{
			SpawnSingleTile(Paths[CurPath].Tiles[CurTile].Row, Paths[CurPath].Tiles[CurTile].Col);
		}
	}
}

void AMaze::SpawnRooms()
{
	for (uint16 CurRoom = 0; CurRoom < Rooms.Num(); ++CurRoom)
	{
		for (uint16 Row = 0; Row < Rooms[CurRoom].High; ++Row)
		{
			for (uint16 Col = 0; Col < Rooms[CurRoom].Wide; ++Col)
			{
				SpawnSingleTile(Rooms[CurRoom].OriginRow + Row, Rooms[CurRoom].OriginCol + Col);
			}
		}
	}
}

void AMaze::SpawnSingleTile(uint16 Row, uint16 Col)
{
	//Location of tile to be placed
	FVector TileLoc;

	float x, y, z;
	z = 0;
	x = Row * 500;
	y = Col * 500;
	TileLoc = FVector(x, y, z);
	FRotator Rotation = FRotator(0, 0, 0);

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;
	SpawnInfo.Owner = this;
	SpawnInfo.bDeferConstruction = false;

	UWorld* const World = GetWorld();
	if (World)
	{
		ATile* const SpawnedTile = World->SpawnActor<ATile>(BlueprintVar, TileLoc, Rotation, SpawnInfo);
	}

	//AMaze* NewTile = World->SpawnActor(TileBP, &TileLoc, &Rotation, SpawnInfo);
}
