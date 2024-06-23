#include "MaterialDetailEditor.h"
#include "FormMain.h"
#include <MaterialEditor.h>
#include <qlayout.h>
#include <qboxlayout.h>
#include <EditorCommonFunction.h>
#include <AssetLine.h>
#include <ContentBrowser.h>
#include <VulkanRenderer.h>
MaterialDetailEditor::MaterialDetailEditor(std::weak_ptr<Material> mat, QWidget *parent)
	: QMainWindow(parent)
{
	QWidget* r = new QWidget(this);
	QWidget* ma = new QWidget(this);
	QWidget* mp = new QWidget(this);
	ui_r.setupUi(r);
	ui_ma.setupUi(ma);
	ui_mp .setupUi(mp);
	_parent = (MaterialEditor*)parent;
	_material = mat;

	left_right = new QSplitter(this);
	left_right->setOrientation(Qt::Orientation::Horizontal);
	QWidget* widget = new QWidget(this);
	widget->setLayout(new QVBoxLayout(this));
	((QVBoxLayout*)widget->layout())->setContentsMargins(0,0,0,0);
	((QVBoxLayout*)widget->layout())->setSpacing(0);
	left_right->addWidget(widget);
	left_right->addWidget(mp);
	//
	QSplitter* up_bottom = new QSplitter(this);
	up_bottom->setOrientation(Qt::Orientation::Vertical);
	widget->layout()->addWidget(up_bottom);
	up_bottom->addWidget(r);
	up_bottom->addWidget(ma);
	//Ð¡±êÌâ
	ui_r.MaterialDetailEditor_RendererName->setText(GetEditorInternationalization("MaterialEditor","MaterialRendererTitle"));
	ui_mp.MaterialDetailEditor_MaterialParameterName->setText(GetEditorInternationalization("MaterialEditor", "MaterialParameterTitle"));
	ui_ma.MaterialDetailEditor_MaterialAttributeName->setText(GetEditorInternationalization("MaterialEditor", "MaterialAttrbuteTitle"));
	//äÖÈ¾Æ÷
	_matWindow = VulkanApp::CreateNewWindow(512, 512, _material.lock()->_assetInfo.lock()->guid.str().c_str(), true, (void*)ui_r.MaterialDetailEditor_RenderView->winId());
	//HWND hwnd = (HWND)VulkanApp::GetWindowHandle(_matWindow);
	//Material attribute
	{
		//path
		{
			AssetLine* path = new AssetLine("VPath", this, "");
			ui_ma.scroll_verticalLayout->addWidget(path);
			path->ui.LineEdit->setReadOnly(true);
			path->ui.LineEdit->setText(_material.lock()->_assetInfo.lock()->virtualFilePath.c_str());
			path->ui.picture->setHidden(true);
			path->ui.pushButton->setHidden(true);
			path->_bindFindButtonFunc = [this](const char* p) {
				auto assetInfo = _material.lock()->_assetInfo;
				if (!assetInfo.expired())
				{
					auto treeItems = assetInfo.lock()->virtualPath.Split("/");
					QString path;
					CustomViewItem* treeItem = nullptr;
					QModelIndex treeIndex;
					for (auto& i : treeItems)
					{
						path += ("/" + i).c_str();
						treeItem = ContentBrowser::GetCurrentBrowser()->_treeView->FindItem(path);
						if (treeItem)
						{
							ContentBrowser::GetCurrentBrowser()->_treeView->_bSaveSelectionItem = true;
							treeIndex = ((QStandardItemModel*)ContentBrowser::GetCurrentBrowser()->_treeView->model())->indexFromItem(treeItem);
							ContentBrowser::GetCurrentBrowser()->_treeView->expand(treeIndex);
						}
					}
					if (treeIndex.isValid())
					{
						ContentBrowser::GetCurrentBrowser()->_treeView->selectionModel()->setCurrentIndex(treeIndex, QItemSelectionModel::SelectionFlag::ClearAndSelect);
					}
					auto item = ContentBrowser::GetCurrentBrowser()->_listView->FindAssetItem(assetInfo.lock()->guid);
					if (item)
					{
						ContentBrowser::GetCurrentBrowser()->_listView->scrollToItem(item);
						ContentBrowser::GetCurrentBrowser()->_listView->setCurrentItem(item, QItemSelectionModel::SelectionFlag::ClearAndSelect);
					}
				}
			};
		}

		ui_ma.scroll_verticalLayout->addStretch(20);
	}

}

MaterialDetailEditor::~MaterialDetailEditor()
{
}

void MaterialDetailEditor::resizeEvent(QResizeEvent * event)
{
	left_right->resize(width(), height());
}

void MaterialDetailEditor::closeEvent(QCloseEvent* event)
{
	VulkanApp::RemoveWindow(_matWindow);
	for (int i = 0 ; i < _parent->_allDetailWindows.size() ; i++)
	{
		if (_parent->_allDetailWindows[i].editor == this)
		{
			_parent->_allDetailWindows.removeAt(i);
			break;
		}
	}
}
