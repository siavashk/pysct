//
//  Image3D.cpp
//  Test
//
//  Created by benji_admin on 2013-09-19.
//  Copyright (c) 2013 benji_admin. All rights reserved.
//

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
//
// ITK Headers
//
#include "itkMesh.h"
#include "itkLineCell.h"
#include "itkTriangleCell.h"



//
// VTK headers
//
#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include <vtkFillHolesFilter.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkPointData.h>

#include "Image3D.h"
#include "../util/Matrix3x3.h"
#include "OrientImage.h"

using namespace std;

typedef itk::Image< double, 3 > ImageType;
typedef itk::Image< unsigned char, 3 > BinaryImageType;
typedef itk::CovariantVector<double,3> PixelType;
typedef itk::Image< PixelType, 3 > ImageVectorType;
typedef ImageVectorType::IndexType IndexType;
typedef BinaryImageType::IndexType BinaryIndexType;
typedef itk::Point< double, 3 > PointType;

typedef itk::CastImageFilter< ImageType, BinaryImageType > CastFilterType;

//typedef itk::Mesh< double, 3 > MeshTypeB;

const unsigned int PointDimension   = 3;
const unsigned int MaxCellDimension = 2;
typedef itk::DefaultStaticMeshTraits<
                      double,
                      PointDimension,
                      MaxCellDimension,
                      double,
                      double  >                     MeshTraits;


typedef itk::Mesh<
                      double,
                      PointDimension,
                      MeshTraits              >     MeshTypeB;
typedef MeshTypeB::CellType		CellInterfaceB;
typedef itk::TriangleCell<CellInterfaceB> CellTypeB;
typedef itk::TriangleMeshToBinaryImageFilter<MeshTypeB, BinaryImageType> MeshFilterType;

typedef itk::LinearInterpolateImageFunction< ImageType, double > InterpolateIntensityFilter;
typedef itk::ContinuousIndex<double, 3> ContinuousIndexType;
typedef itk::VectorLinearInterpolateImageFunction< ImageVectorType, double > InterpolateVectorFilter;

typedef itk::SpatialOrientation::ValidCoordinateOrientationFlags OrientationType;



Image3D::Image3D(ImageVectorType::Pointer im, int hauteur, int largeur, int profondeur, CVector3 origine, CVector3 directionX, CVector3 directionY, CVector3 directionZ, CVector3 spacing, double typeImageFactor)
{
    image_ = im;
    hauteur_ = hauteur;
    largeur_ = largeur;
    profondeur_ = profondeur;
    origine_ = origine;
    directionX_ = directionX;
    directionY_ = directionY;
    directionZ_ = directionZ;
    spacing_ = spacing;
    type_image_factor_ = typeImageFactor;
    
    direction[0] = directionX_[0]; direction[3] = directionX_[1]; direction[6] = directionX_[2];
    direction[1] = directionY_[0]; direction[4] = directionY_[1]; direction[7] = directionY_[2];
    direction[2] = directionZ_[0]; direction[5] = directionZ_[1]; direction[8] = directionZ_[2];
    
    directionInverse = direction.Inverse();
    
    extremePoint_ = CVector3(spacing_[0]*hauteur_,spacing_[1]*largeur_,spacing_[2]*profondeur_);
    
    imageInterpolator = InterpolateIntensityFilter::New();
    vectorImageInterpolator = InterpolateVectorFilter::New();
    vectorImageInterpolator->SetInputImage(image_);
    
    boolImageMagnitudeGradient_ = false;
    boolImageOriginale_ = false;
    boolCroppedOriginalImage_ = false;
    boolImage_ = true;
    boolLaplacianImage_ = false;
}

float Image3D::GetPixelOriginal(const CVector3& index)
{
    IndexType ind = {static_cast<itk::IndexValueType>(index[0]),static_cast<itk::IndexValueType>(index[1]),static_cast<itk::IndexValueType>(index[2])};
    return croppedOriginalImage_->GetPixel(ind);
}

float Image3D::GetPixelMagnitudeGradient(const CVector3& index)
{
    IndexType ind = {static_cast<itk::IndexValueType>(index[0]),static_cast<itk::IndexValueType>(index[1]),static_cast<itk::IndexValueType>(index[2])};
    return imageMagnitudeGradient_->GetPixel(ind);
}

float Image3D::GetContinuousPixelMagnitudeGradient(const CVector3& index)
{
    ContinuousIndexType ind; ind[0] = index[0]; ind[1] = index[1]; ind[2] = index[2];
    return imageInterpolator->EvaluateAtContinuousIndex(ind);
}

PixelType Image3D::GetPixel(const CVector3& index)
{
    IndexType ind = {static_cast<itk::IndexValueType>(index[0]),static_cast<itk::IndexValueType>(index[1]),static_cast<itk::IndexValueType>(index[2])};
    return image_->GetPixel(ind);
}

CVector3 Image3D::GetPixelVector(const CVector3& index)
{
    IndexType ind = {static_cast<itk::IndexValueType>(index[0]),static_cast<itk::IndexValueType>(index[1]),static_cast<itk::IndexValueType>(index[2])};
    PixelType pixel = image_->GetPixel(ind);
    return CVector3(pixel[0],pixel[1],pixel[2]);
}

CVector3 Image3D::GetContinuousPixelVector(const CVector3& index)
{
    ContinuousIndexType ind; ind[0] = index[0]; ind[1] = index[1]; ind[2] = index[2];
    PixelType pixel = vectorImageInterpolator->EvaluateAtContinuousIndex(ind);
    return CVector3(pixel[0],pixel[1],pixel[2]);
}

CVector3 Image3D::GetPixelVectorLaplacian(const CVector3& index)
{
    IndexType ind = {static_cast<itk::IndexValueType>(index[0]),static_cast<itk::IndexValueType>(index[1]),static_cast<itk::IndexValueType>(index[2])};
    PixelType pixel = laplacianImage_->GetPixel(ind);
    return CVector3(pixel[0],pixel[1],pixel[2]);
}

bool Image3D::TransformPhysicalPointToIndex(const CVector3& point, CVector3& index)
{
    PointType pt; pt[0] = point[0]; pt[1] = point[1]; pt[2] = point[2];
    IndexType ind;
    bool result = image_->TransformPhysicalPointToIndex(pt,ind);
    index[0] = ind[0]; index[1] = ind[1]; index[2] = ind[2];
    return result;
}

bool Image3D::TransformPhysicalPointToContinuousIndex(const CVector3& point, CVector3& index)
{
    PointType pt; pt[0] = point[0]; pt[1] = point[1]; pt[2] = point[2];
    ContinuousIndexType ind;
    bool result = image_->TransformPhysicalPointToContinuousIndex(pt,ind);
    index[0] = ind[0]; index[1] = ind[1]; index[2] = ind[2];
    return result;
}

CVector3 Image3D::TransformIndexToPhysicalPoint(const CVector3& index)
{
    IndexType ind = {static_cast<itk::IndexValueType>(index[0]),static_cast<itk::IndexValueType>(index[1]),static_cast<itk::IndexValueType>(index[2])};
    PointType point;
    image_->TransformIndexToPhysicalPoint(ind,point);
    return CVector3(point[0],point[1],point[2]);
}

CVector3 Image3D::TransformContinuousIndexToPhysicalPoint(const CVector3& index)
{
    ContinuousIndexType ind; ind[0] = index[0]; ind[1] = index[1]; ind[2] = index[2];
    PointType point;
    image_->TransformContinuousIndexToPhysicalPoint(ind,point);
    return CVector3(point[0],point[1],point[2]);
}

double Image3D::GetMaximumNorm()
{
    ImageVectorType::RegionType region = image_->GetLargestPossibleRegion();
    itk::ImageRegionConstIterator<ImageVectorType> imageIterator(image_,region);
    PixelType pixelGradient;
    double max = 0.0, courant;
    while(!imageIterator.IsAtEnd())
    {
        pixelGradient = imageIterator.Get();
        courant = pixelGradient.GetNorm();
        if (courant > max) max = courant;
        
        ++imageIterator;
    }
    return max;
}

void Image3D::NormalizeByMaximum()
{
    double max = GetMaximumNorm();
    
    ImageVectorType::RegionType region = image_->GetLargestPossibleRegion();
    itk::ImageRegionConstIterator<ImageVectorType> imageIterator(image_,region);
    PixelType pixelGradient;
    IndexType indexGradient;
    while(!imageIterator.IsAtEnd())
    {
        indexGradient = imageIterator.GetIndex();
        pixelGradient = imageIterator.Get();
        image_->SetPixel(indexGradient,(pixelGradient*2)/max);
        ++imageIterator;
    }
}

void Image3D::DeleteHighVector()
{
    double threshold = GetMaximumNorm()/3;
    
    ImageVectorType::RegionType region = image_->GetLargestPossibleRegion();
    itk::ImageRegionConstIterator<ImageVectorType> imageIterator(image_,region);
    PixelType pixelGradient;
    IndexType indexGradient;
    while(!imageIterator.IsAtEnd())
    {
        indexGradient = imageIterator.GetIndex();
        pixelGradient = imageIterator.Get();
        if (pixelGradient.GetNorm() > threshold) {
            pixelGradient[0] = 0.0; pixelGradient[1] = 0.0; pixelGradient[2] = 0.0;
            image_->SetPixel(indexGradient,pixelGradient);
        }
        ++imageIterator;
    }
}

BinaryImageType::Pointer Image3D::TransformMeshToBinaryImage(Mesh* m)
{
    MeshTypeB::Pointer mesh;
    MeshFilterType::Pointer meshFilter = MeshFilterType::New();
    
	mesh = MeshTypeB::New();
	vector<Vertex*> points = m->getListPoints();
	PointType pnt;
	CVector3 p, n;
	for (unsigned int i = 0; i < points.size(); i++) {
		p = points[i]->getPosition();
		n = points[i]->getNormal();
		pnt[0] = p[0]; pnt[1] = p[1]; pnt[2] = p[2];
		mesh->SetPoint(i, pnt);
	}
	vector<int> triangles = m->getListTriangles();
	for (unsigned int i = 0; i < triangles.size(); i += 3)
	{
		CellTypeB::CellAutoPointer triangle;
		triangle.TakeOwnership(new CellTypeB);
		triangle->SetPointId(0, triangles[i]);
		triangle->SetPointId(1, triangles[i + 1]);
		triangle->SetPointId(2, triangles[i + 2]);
		mesh->SetCell((int)(i + 1) / 3, triangle);
	}
	meshFilter->SetInput(mesh);

    meshFilter->SetOrigin(imageOriginale_->GetOrigin());
    meshFilter->SetSpacing(imageOriginale_->GetSpacing());
    meshFilter->SetSize(imageOriginale_->GetLargestPossibleRegion().GetSize());
    meshFilter->SetDirection(imageOriginale_->GetDirection());
    meshFilter->SetIndex(imageOriginale_->GetLargestPossibleRegion().GetIndex());
    meshFilter->SetInsideValue(1.0);
    meshFilter->SetOutsideValue(0.0);

    try 
    {
        meshFilter->Update();
    }
    catch( itk::ExceptionObject & e )
    {
        cout << "Exception thrown ! " << endl;
        cout << "An error ocurred during creating binary image" << endl;
        cout << "Location    = " << e.GetLocation()    << endl;
        cout << "Description = " << e.GetDescription() << endl;
    }
    
    return meshFilter->GetOutput();
}

void Image3D::setImageOriginale(ImageType::Pointer i)
{
    imageOriginale_ = i;
    boolImageOriginale_ = true;
}

void Image3D::setCroppedImageOriginale(ImageType::Pointer i)
{
    croppedOriginalImage_ = i;
    boolCroppedOriginalImage_ = true;
}

void Image3D::setImageMagnitudeGradient(ImageType::Pointer i)
{
    imageMagnitudeGradient_ = i;
    imageInterpolator->SetInputImage(imageMagnitudeGradient_);
    boolImageMagnitudeGradient_ = true;
}

void Image3D::setLaplacianImage(ImageVectorType::Pointer i)
{
    laplacianImage_ = i;
    boolLaplacianImage_ = true;
}


void Image3D::releaseMemory()
{
    if (boolImageMagnitudeGradient_) imageMagnitudeGradient_->Delete();
    if (boolImage_) image_->Delete();
    if (boolLaplacianImage_) laplacianImage_->Delete();
    boolImageMagnitudeGradient_ = false;
    boolImageOriginale_ = false;
    boolLaplacianImage_ = false;
}
