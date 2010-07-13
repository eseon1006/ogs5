/**
 * \file VtkVisPipelineView.h
 * 18/2/2010 LB Initial implementation
 *
 */


#ifndef VTKVISPIPELINEVIEW_H
#define VTKVISPIPELINEVIEW_H

// ** INCLUDES **
#include <QTreeView>

class QItemSelection;
class VtkVisPipelineItem;
class vtkActor;

/**
 * VtkVisPipelineView is a QTreeView and shows VtkVisPipelineItems.
 */
class VtkVisPipelineView : public QTreeView
{
	Q_OBJECT

public:
	VtkVisPipelineView(QWidget* parent = 0);

protected slots:
	/// Emits itemSelected() signals when an items was selected.
	void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
	void selectItem(vtkActor* actor);

private:
	/// Creates a menu on right-clicking on an item.
	void contextMenuEvent(QContextMenuEvent* event);

private slots:

	void exportSelectedPipelineItem();

	/// Sends an requestRemovePipelineItem() signal to remove
	/// the currently selected item.
	void removeSelectedPipelineItem();

	/// Sends a requestAddPipelineFilterItem() signal to add
	/// a filter.
	void addPipelineFilterItem();

signals:
	void requestRemovePipelineItem(QModelIndex);
	void requestAddPipelineFilterItem(QModelIndex);
	void itemSelected(VtkVisPipelineItem*);
	void actorSelected(vtkActor*);

};

#endif // VTKVISPIPELINEVIEW_H
