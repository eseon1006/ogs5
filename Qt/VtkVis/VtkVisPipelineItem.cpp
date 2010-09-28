/**
 * \file VtkVisPipelineItem.cpp
 * 17/2/2010 LB Initial implementation
 *
 * Implementation of VtkVisPipelineItem
 */

// ** INCLUDES **
#include "VtkVisPipelineItem.h"
#include "VtkAlgorithmProperties.h"

#include <vtkAlgorithm.h>
#include <vtkPointSet.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>

#include <QMessageBox>

#ifdef OGS_USE_OPENSG
#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGCoredNodePtr.h>
#include <OpenSG/OSGGroup.h>
#include "vtkOsgActor.h"
#endif

// export test
#include <vtkPolyDataAlgorithm.h>
#include <vtkPolyDataWriter.h>
#include <vtkUnstructuredGridAlgorithm.h>
#include <vtkUnstructuredGridWriter.h>

#ifdef OGS_USE_OPENSG
	VtkVisPipelineItem::VtkVisPipelineItem(
		vtkRenderer* renderer,
		vtkAlgorithm* algorithm,
		TreeItem* parentItem,
		vtkPointSet* input,
		OSG::NodePtr parentNode,
		const QList<QVariant> data /*= QList<QVariant>()*/)
	: TreeItem(data, parentItem), _algorithm(algorithm), _input(input), _renderer(renderer), _parentNode(parentNode)
	{
		//if (_input != NULL)
			//_algorithm->SetInput(_input);
			//static_cast<vtkPolyDataAlgorithm*>(_algorithm)->SetInput(_input);

		Initialize();
	}
#else // OGS_USE_OPENSG
	VtkVisPipelineItem::VtkVisPipelineItem(
		vtkRenderer* renderer,
		vtkAlgorithm* algorithm,
		TreeItem* parentItem,
		vtkPointSet* input,
		const QList<QVariant> data /*= QList<QVariant>()*/)
	: TreeItem(data, parentItem), _algorithm(algorithm), _input(input), _renderer(renderer)
	{
		//if (_input != NULL)
			//_algorithm->SetInput(_input);
			//static_cast<vtkPolyDataAlgorithm*>(_algorithm)->SetInput(_input);

		Initialize();
	}
#endif // OGS_USE_OPENSG


VtkVisPipelineItem::~VtkVisPipelineItem()
{
	#ifdef OGS_USE_OPENSG
		vtkOsgActor* osgActor = dynamic_cast<vtkOsgActor*>(actor());
		if(_parentNode != NullFC)
		{
			OSG::beginEditCP(_parentNode);{
				_parentNode->subChild(osgActor->GetOsgRoot());
			};OSG::endEditCP(_parentNode);
		}
		_renderer->RemoveActor(osgActor);
		_mapper->Delete();
		osgActor->Delete();
	#else // OGS_USE_OPENSG
		_renderer->RemoveActor(_actor);
		_mapper->Delete();
		_actor->Delete();
	#endif // OGS_USE_OPENSG
}

QVariant VtkVisPipelineItem::data( int column ) const
{
	if (column == 1)
	{
		return isVisible();
	}
	else
		return TreeItem::data(column);
}

bool VtkVisPipelineItem::setData( int column, const QVariant &value )
{
	if (column == 1)
	{
		setVisible(value.toBool());
		return true;
	}
	else
		return TreeItem::setData(column, value);

}
bool VtkVisPipelineItem::isVisible() const
{
	return (bool)_actor->GetVisibility();
}

void VtkVisPipelineItem::setVisible( bool visible )
{
	_actor->SetVisibility((int)visible);
	_actor->Modified();
	_renderer->Render();
}

void VtkVisPipelineItem::Initialize()
{
	_mapper = vtkDataSetMapper::New();
	_mapper->SetInputConnection(0, _algorithm->GetOutputPort(0));
#ifdef OGS_USE_OPENSG
	_actor = vtkOsgActor::New();
	_actor->SetMapper(_mapper);
	_actor->Render(_renderer, _mapper);
	//actor->SetVerbose(true);
	_actor->UpdateOsg();

	OSG::beginEditCP(_parentNode);{
		_parentNode->addChild(_actor->GetOsgRoot());
	};OSG::endEditCP(_parentNode);
#else
	_actor = vtkActor::New();
	_actor->SetMapper(_mapper);
#endif
	_renderer->AddActor(_actor);

	// Set pre-set properties
	VtkAlgorithmProperties* vtkProps = dynamic_cast<VtkAlgorithmProperties*>(_algorithm);
	if (vtkProps)
		setVtkProperties(vtkProps);

	// Copy properties from parent
	else
	{
		VtkVisPipelineItem* parentItem = dynamic_cast<VtkVisPipelineItem*>(parent());
		while (parentItem)
		{
			VtkAlgorithmProperties* parentProps = dynamic_cast<VtkAlgorithmProperties*>(parentItem->algorithm());
			if (parentProps)
			{
				setVtkProperties(parentProps);
				parentItem = NULL;
			}
			else
				parentItem = dynamic_cast<VtkVisPipelineItem*>(parentItem->parent());
		}
	}

}

void VtkVisPipelineItem::setVtkProperties(VtkAlgorithmProperties* vtkProps)
{
	if (vtkProps->GetTexture() != NULL)
	{
		_mapper->ScalarVisibilityOff();
		_actor->GetProperty()->SetColor(1,1,1); // don't colorise textures
		_actor->SetTexture(vtkProps->GetTexture());
	}
	else
	{
		vtkSmartPointer<vtkProperty> itemProperty = vtkProps->GetProperties();
		_actor->SetProperty(itemProperty);
	}

	if (!vtkProps->GetScalarVisibility())
		_mapper->ScalarVisibilityOff();
}

int VtkVisPipelineItem::writeToFile(const std::string &filename) const
{	
	if (!filename.empty())
	{
		if (filename.substr(filename.size() - 4).find("os") != std::string::npos)
		{
#ifdef OGS_USE_OPENSG
			vtkOsgActor* osgActor = static_cast<vtkOsgActor*>(_actor);
			osgActor->SetVerbose(true);
			osgActor->UpdateOsg();
			OSG::SceneFileHandler::the().write(osgActor->GetOsgRoot(), filename.c_str());
			osgActor->ClearOsg();
#else
			QMessageBox::warning(NULL, "Functionality not implemented",
				"Sorry but this progam was not compiled with OpenSG support.");
#endif
		}
		else
		{
			vtkAlgorithm* alg = this->algorithm();
			vtkPolyDataAlgorithm* algPD = dynamic_cast<vtkPolyDataAlgorithm*>(alg);
			if (algPD)
			{
				vtkSmartPointer<vtkPolyDataWriter> pdWriter = vtkSmartPointer<vtkPolyDataWriter>::New();
				pdWriter->SetInput(algPD->GetOutputDataObject(0));
				pdWriter->SetFileName(filename.c_str());
				int result = pdWriter->Write();
				return result;
			}
			else
			{
				vtkUnstructuredGridAlgorithm* algUG = dynamic_cast<vtkUnstructuredGridAlgorithm*>(alg);
				if (algUG)
				{
					std::string gridName(filename);
					gridName.replace(filename.length()-1, 1, "u"); // change fileextension from "vtp" to "vtu"
					vtkSmartPointer<vtkUnstructuredGridWriter> ugWriter = vtkSmartPointer<vtkUnstructuredGridWriter>::New();
					ugWriter->SetInput(algUG->GetOutputDataObject(0));
					ugWriter->SetFileName(filename.c_str());
					int result = ugWriter->Write();
					return result;
				}
			}
			std::cout << "VtkVisPipelineItem::writeToFile() - Unknown data type..." << std::endl;
		}
	}
	return 0;
}

vtkActor* VtkVisPipelineItem::actor() const
{
	return static_cast<vtkActor*>(_actor);
}