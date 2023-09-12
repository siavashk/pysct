/*! \file Main.cpp
 * \mainpage sct_propseg
 *
 * \section description Description
 * Part of the Spinal Cord Toolbox <https://sourceforge.net/projects/spinalcordtoolbox>
 *
 * This program segments automatically the spinal cord on T1- and T2-weighted images, for any field of view. You must provide the type of contrast and the image.
 *
 * Primary output is the binary mask of the spinal cord segmentation (a voxel is inside the spinal cord when its center is inside the segmentation surface). This method must provide VTK triangular mesh of the segmentation (option -mesh). Spinal cord centerline is available as a binary image (-centerline-binary) or a text file with coordinates in world referential (-centerline-coord). It also provide the cross-sectional areas of the spinal cord, for each "z" slice.
 *
 * Several tips on segmentation correction can be found on the \ref correction_tips "Correction Tips" page of the documentation while advices on parameters adjustments can be found on the \ref parameters_adjustment "Parameters" page.
 *
 * If the segmentation fails at some location (e.g. due to poor contrast between spinal cord and CSF), edit your anatomical image (e.g. with fslview) and manually enhance the contrast by adding bright values around the spinal cord for T2-weighted images (dark values for T1-weighted). Then, launch the segmentation again.
 *
 * \section usage Usage
 * \code sct_propseg -i <inputfilename> -o <outputfolderpath> -t <imagetype> [options] \endcode
 *
 * \section input Input parameters
 *
 * MANDATORY ARGUMENTS:
 *		* -i <inputfilename>            (no default)
 *		* -t <imagetype> {t1,t2}        (string, type of image contrast, t2: cord dark / CSF bright ; t1: cord bright / CSF dark, no default)
 *		* -help
 *

 *
 * \section dep Dependencies
 * - ITK (http://www.itk.org/)
 * - VTK (http://www.vtk.org/)
 *
 * \section com Comments
 * Copyright (c) 2014 NeuroPoly, Polytechnique Montreal \<http://www.neuro.polymtl.ca\>
 *
 * Author: Benjamin De Leener
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 *
 */
#define _SCL_SECURE_NO_WARNINGS

 // std libraries
#include <iostream>
#include <string>
#include <stdlib.h>
#include <time.h>

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkNiftiImageIO.h>

#include "../propseg/SegmentationPropagation.h"


std::string makeOutputFilename(const char* filename) {
    // Convert the const char pointer to a std::string
    std::string strFilename(filename);

    // Find the last occurrence of the dot ('.') character
    size_t dotPosition = strFilename.find('.');

    // If a dot is found and it's not the first character
    if (dotPosition != std::string::npos && dotPosition != 0) {
        // Extract the substring before the dot
        const std::string filenameWithoutExtension = strFilename.substr(0, dotPosition).c_str();
        const std::string outputFilename = filenameWithoutExtension + "_spinalcord.nii.gz";
        return outputFilename;
    }

    // If no dot was found or it's at the beginning, return the original string
    return strFilename;
}


int main(int argc, char* argv[])
{
    srand(time(NULL));

    if (argc != 2)
    {
        return EXIT_FAILURE;
    }

    std::string outputFilename = makeOutputFilename(argv[1]);

    typedef itk::Image< double, 3 >	ImageType;
    typedef itk::ImageFileReader<ImageType> ReaderType;
    typedef itk::ImageFileWriter<BinaryImageType> WriterType;

    ReaderType::Pointer reader = ReaderType::New();
    itk::NiftiImageIO::Pointer io = itk::NiftiImageIO::New();
    reader->SetImageIO(io);
    reader->SetFileName(argv[1]);

    try
    {
        reader->Update();
    }
    catch (itk::ExceptionObject& err)
    {
        std::cerr << "Exception caught while reading the input image" << std::endl;
        std::cerr << argv[1] << std::endl;
        std::cerr << err << std::endl;

        return EXIT_FAILURE;
    }

    SegmentationPropagation segmentationPropagation;

    BinaryImageType::Pointer spinalCord = segmentationPropagation.run(reader->GetOutput());

    WriterType::Pointer writer = WriterType::New();
    writer->SetInput(spinalCord);
    writer->SetFileName(outputFilename.c_str());

    try
    {
        writer->Update();
    }
    catch (itk::ExceptionObject& err)
    {
        std::cerr << "Exception caught while writing the output spinal cord segmentation" << std::endl;
        std::cerr << outputFilename << std::endl;
        std::cerr << err << std::endl;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}