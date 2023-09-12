//
//  SegmentationPropagation.h
//  sct_segmentation_propagation
//
//  Created by Benjamin De Leener on 2014-04-16.
//  Refactored by Siavash Khallaghi 2023-09-7.
//  Copyright (c) 2014 Benjamin De Leener. All rights reserved.
//

#ifndef __sct_segmentation_propagation__SegmentationPropagation__
#define __sct_segmentation_propagation__SegmentationPropagation__

#include <itkImage.h>
#include <itkGradientMagnitudeImageFilter.h>
#include <itkGradientImageFilter.h>
#include <itkIntensityWindowingImageFilter.h>
#include <itkMedianImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

#include <vtkSmartPointer.h>
#include <vtkOutputWindow.h>
#include <vtkObject.h>

#include "Initialisation.h"
#include "Image3D.h"
#include "OrientImage.h"
#include "PropagatedDeformableModel.h"


using ImageType = itk::Image< double, 3 >;
using MedianFilterType = itk::MedianImageFilter< ImageType, ImageType >;
using MinMaxCalculatorType = itk::MinimumMaximumImageCalculator< ImageType >;
using RescaleFilterType = itk::IntensityWindowingImageFilter< ImageType, ImageType >;
using GradientPixelType = itk::CovariantVector< double, 3 >;
using GradientImageType = itk::Image< GradientPixelType, 3 >;
using GradientMagnitudeFilterType = itk::GradientMagnitudeImageFilter< ImageType, ImageType >;
using VectorGradientFilterType = itk::GradientImageFilter< ImageType, float, double, GradientImageType >;


class SegmentationPropagation 
{
public:
	SegmentationPropagation();
	~SegmentationPropagation() {};

	BinaryImageType::Pointer run(ImageType::Pointer image);

private:
	void performInitialization(ImageType::Pointer image);
	std::unique_ptr<Image3D> makeImage3D(ImageType::Pointer image);

	MedianFilterType::Pointer medianFilter_;
	MinMaxCalculatorType::Pointer minMaxCalculator_;
	RescaleFilterType::Pointer rescaleFilter_;

	GradientMagnitudeFilterType::Pointer gradientMagnitudeFilterPointer_;
	VectorGradientFilterType::Pointer gradientMapFilterPointer_;

	std::unique_ptr<OrientImage<ImageType>> orientationFilterPointer_;

	std::unique_ptr<Initialisation> initialisationPointer_;
	std::unique_ptr<PropagatedDeformableModel> propagtedDeformableModelPointer_;
	
	bool isSpinalCordDetected_;
	CVector3 point_, normal1_, normal2_;
	
	double radius_ = 4.0;
	const int gapInterSlices_ = 4;
	const int nbSlicesInitialisation_ = 5;
	const double initialisation_ = 0.5;
	const double typeImageFactor_ = 1.0; // T2 image for T1 it is -1.0
	double stretchingFactor_ = 1.0;

	const double minContrast_ = 50.0;
	const int downSlice_ = -10000;
	const int upSlice_ = 10000;

	const int radialResolution_ = 15;
	const int axialResolution_ = 3;
	const int numberOfDeformIteration_ = 3;
	const int numberOfPropagationIteration_ = 200;
	const double axialStep_ = 6.0; 
	const double propagationLength_ = 800.0;
};

#endif 