// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Tile.h"
#include "Maze.generated.h"

USTRUCT(BlueprintType)
struct CANYONS_API FMazeRow
{
	GENERATED_USTRUCT_BODY()

		//row[i].col[i] is true if place is filled, false if place is empty
		UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Maze")
		TArray<bool> Col;

	FMazeRow(){}
};

USTRUCT(BlueprintType)
struct CANYONS_API FMazeArray
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Maze")
		TArray<FMazeRow> Row;

	FMazeArray(){}

};

USTRUCT()
struct CANYONS_API FRoom
{
	GENERATED_USTRUCT_BODY()

	uint16 OriginRow, OriginCol, High, Wide, SetID;
	//uint16 EntranceRow, EntranceCol; Rooms used to save their entrance tile, now it is part of the paths

	//Checks if the room contains a specified square
	bool Contains(uint16 Row, uint16 Col)
	{
		if (Row >= OriginRow && Row < OriginRow + High && Col >= OriginCol && Col < OriginCol + Wide)
		{
			return true;
		}
		return false;
	}
	
	//You can't have a modified constructor with USTRUCTS, so I'm just leaving the data entry to the Room creator, potential error point for sure
	FRoom(){}
};

USTRUCT()
struct CANYONS_API FWalkway
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Maze")
	uint16 Row;
	UPROPERTY(VisibleAnywhere, Category = "Maze")
	uint16 Col;

	//== Operator, needed for TArray remove()
	bool operator==(const FWalkway &other) const
	{
		if (Row == other.Row && Col == other.Col)
		{
			return true;
		}
		return false;
	}

	//You can't have a modified constructor with USTRUCTS, so I'm just leaving the data entry to the Room creator, potential error point for sure
	FWalkway(){}
};

USTRUCT(BlueprintType)
struct CANYONS_API FPath
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Maze")
	TArray<FWalkway> Tiles;

	uint16 SetID;

	FPath(){}
};


UCLASS()
class CANYONS_API AMaze : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	// Sets default values for this actor's properties
	AMaze();

	//Secondary constructor removed until needed again
	//AMaze(const FObjectInitializer &ObjectInitializer, uint16 High, uint16 Wide);

	//Initializes the data structures
	void Initialize();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	//generation functions
	void GenerateMaze();

	void GenerateRooms(uint16 Tries); //Places rooms of random size in the maze, makes Tries attempts
	void PlaceRoom(uint16 OriginRow, uint16 OriginCol, uint16 High, uint16 Wide); //Places a room High tall and Wide wide with the upper left corner at (OriginRow, OriginCol), does nothing if room would connect with another open space

	void DepthFirstMaze(uint16 OriginRow, uint16 OriginCol); //Generates a maze using Depth-first algorith starting at origin(X,Y)
	void DepthFirstMazeHelper(uint16 OriginRow, uint16 OriginCol, uint16 PathIndex);
	void PrimmMaze(uint16 OriginRow, uint16 OriginCol); //Generates a maze using Primm algorith starting at origin(X,Y)

	void RemoveAllDeadEnds(); //Removes all dead ends from the maze component of the map
	void RemoveAllDeadEndsHelper(uint16 OriginRow, uint16 OriginCol); //Recursively removes a single dead end, calling itself on the dead end it creates upon removing one

	void GenerateEntrances(); //Places paths for entering all rooms from nearby mazes. Will not generate entrances for rooms that aren't adjacent to a path or room
	uint16 FindTileOwner(uint16 OriginRow, uint16 OriginCol, bool &OutIsRoom); //Returns the SetID of the set that contains the tile. Returns  0 if not found.

	//Check if a position is in the maze and not yet filled in. Returns true only if both are true.
	bool CheckAvailable(uint8 OriginRow, uint8 OriginCol)
	{
		if (OriginRow > 0 && OriginRow < Height && OriginCol > 0 && OriginCol < Width)
		{
			if (MyMaze.Row[OriginRow].Col[OriginCol] == 0)
			{
				return true;
			}
		}
		return false;
	}

	//Function that spawns tiles to create the maze
	UFUNCTION(BlueprintCallable, Category = "Maze")
	void SpawnTiles();
	//Function that spawns only the tiles that are Paths
	UFUNCTION(BlueprintCallable, Category = "Maze")
	void SpawnPaths();
	//Function that spawns only the tiles that are Rooms and their entrances
	UFUNCTION(BlueprintCallable, Category = "Maze")
	void SpawnRooms();

	void SpawnSingleTile(uint16 Row, uint16 Col);

	//You should only change BossSize if you are re-making the room
	void UpdateBossSize(uint8 NewSize);

	//The Actor to be spawned by SpawnTiles
	TSubclassOf<ATile> BlueprintVar;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Maze")
	FMazeArray MyMaze;
		UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Maze")
	TArray<FRoom> Rooms;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Maze")
	TArray<FPath> Paths;

	uint16 Height;
	uint16 Width;
	//The ID to be assigned to the next set created. Starts from 1. 0 represents no set containing a tile
	uint16 NextID;
	//Boss room size, defaults to 9x9, Boss room must be square
	uint8 BossSize;
};
