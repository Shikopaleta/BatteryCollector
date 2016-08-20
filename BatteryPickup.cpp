// Fill out your copyright notice in the Description page of Project Settings.

#include "BatteryCollector.h"
#include "BatteryPickup.h"

// Set default values
ABatteryPickup::ABatteryPickup()
{
	GetMesh()->SetSimulatePhysics(true);

	// The base power level of the battery
	BatteryPower = 150.0f;
}

void ABatteryPickup::GetsCollected_Implementation()
{
	// Use the base's pickup behaviour
	Super::GetsCollected_Implementation();

	// Destroy the battery
	Destroy();
}

// Report the power level of the battery
float ABatteryPickup::GetPower()
{	
	return BatteryPower;

	
}