#include "Windows.h"
#include<iostream>


Windows::Windows()
{
}


Windows::~Windows()
{
}

int Windows::LoadFromFile(string filepath){
	QDataStream stream(file);
	bool isRawFile = file->fileName().endsWith(".rawsnapshot");
	if (!isRawFile) {
			return -1;
	}
	CleanWorkSpace();
	if (isRawFile) {
  file->open(QIODevice::WriteOnly);
  uchar* snapshot = file->map(0, file->size());
  remoteProcess_->DecodeData((char*)snapshot, file->size(), false);
  file->unmap(snapshot);
  file->close();
  RemoteDataReceived();
}

return 0;
}

void Windows::RemoteDataReceived() {
	remoteRetryCount_ = 5;

	Crawler crawler;
	auto snapshot = remoteProcess_->GetSnapShot();
	auto packedCrawlerData = new PackedCrawlerData(snapshot);
	crawler.Crawl(*packedCrawlerData, snapshot);
	auto crawled = new CrawledMemorySnapshot();
	crawled->Unpack(*crawled, snapshot, *packedCrawlerData);
	delete packedCrawlerData;

	crawled->name_ = "Snapshot_" + QTime::currentTime().toString("H_m_s");
	ShowSnapshot(crawled);
	Print("Snapshot Received And Unpacked.");
}

void Windows::ShowSnapshot(CrawledMemorySnapshot* crawled) {
	GlobalLogDef::log.append("....");
	auto baseWidget = new QWidget();
	baseWidget->setLayout(new QHBoxLayout());
	auto getTableView = [](QWidget* parent) {
		auto view = new QTableView(parent);
		view->setSortingEnabled(true);
		view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
		view->setSelectionBehavior(QAbstractItemView::SelectRows);
		view->setHorizontalScrollMode(QTableView::ScrollMode::ScrollPerPixel);
		view->setVerticalScrollMode(QTableView::ScrollMode::ScrollPerItem);
		view->verticalHeader()->setEnabled(false);
		view->setMinimumWidth(200);
		view->setWordWrap(false);
		//view->setColumnWidth(0,50); // auto set  row width
		return view;
	};
	auto typeTable = getTableView(baseWidget);
	auto instanceTable = getTableView(baseWidget);

	auto spliter = new QSplitter(baseWidget);
	spliter->addWidget(typeTable);
	spliter->addWidget(instanceTable);
	baseWidget->layout()->addWidget(spliter);

	auto snapshotModel = new UMPTypeGroupModel(crawled, typeTable);
	auto snapshotProxyModel = new UMPTableProxyModel(snapshotModel, typeTable);
	typeTable->setModel(snapshotProxyModel);
	typeTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);//QHeaderView::ResizeMode::Stretch
	typeTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
	typeTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::ResizeToContents);


	auto instanceModel = new UMPThingInMemoryModel(instanceTable);
	auto instanceProxyModel = new UMPTableProxyModel(instanceModel, instanceTable);
	instanceTable->setModel(instanceProxyModel);
	instanceTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);//QHeaderView::ResizeMode::Stretch
	instanceTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::ResizeToContents);
	instanceTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeMode::ResizeToContents);
	instanceTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeMode::ResizeToContents);
	instanceTable->horizontalHeader()->setSectionHidden(3, !crawled->isDiff_);

	auto detailPanel = new DetailsWidget(crawled);
	spliter->addWidget(detailPanel);
	spliter->setCollapsible(0, false);
	spliter->setCollapsible(1, false);
	spliter->setCollapsible(2, false);
	connect(detailPanel, &DetailsWidget::ThingSelected, this, &MainWindow::OnThingSelected);

	connect(typeTable->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selected, const QItemSelection &) {
		if (selected.indexes().size() > 0) {
			auto index = selected.indexes()[0];
			if (index.isValid()) {
				auto row = snapshotProxyModel->mapToSource(index).row();
				auto data = snapshotModel->getSubModel(row);
				instanceTable->clearSelection();
				instanceModel->reset(data, snapshotModel->getSnapshot()->isDiff_);
			}
		}
	});

	connect(instanceTable->selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection &selected, const QItemSelection &) {
		if (selected.indexes().size() > 0) {
			auto index = selected.indexes()[0];
			if (index.isValid()) {
				auto row = instanceProxyModel->mapToSource(index).row();
				auto thing = instanceModel->thingAt(row);
				detailPanel->ShowThing(thing, thing->type());
				if (thing) {
					snapShots_[baseWidget].Push(thing->index_);
					UpdateShowNextPrev();
				}
			}
		}
	});

	SnapshotTabInfo tabInfo;
	tabInfo.stack_ = new QUndoStack(baseWidget);
	tabInfo.snapshot_ = crawled;
	tabInfo.spliter_ = spliter;
	tabInfo.snapshotModel_ = snapshotProxyModel;
	tabInfo.instanceModel_ = instanceProxyModel;
	snapShots_[baseWidget] = tabInfo;

	ui->upperTabWidget->addTab(baseWidget, crawled->name_);
	ui->upperTabWidget->setCurrentWidget(baseWidget);
	ui->upperTabWidget->setTabToolTip(
		ui->upperTabWidget->currentIndex(),
		"Total: " + sizeToString(snapshotModel->getTotalSize()));


	Print(GlobalLogDef::log);
	Print("88888888888888888.");

	QAbstractItemModel *model = typeTable->model();
	QModelIndex index = model->index(3, 3);
	QVariant data = model->data(index);
	QString str = "";
	str.append("No,Type,Count,Size\r\n");
	for (int i = 0; i < model->rowCount(); i++) {
		for (int j = 0; j < model->columnCount(); j++) {
			index = model->index(i, j);
			str.append(model->data(index).toString());
			if (j != model->columnCount() - 1)str.append(",");
		}
		str.append("\r\n");
	}
	ui->consolePlainTextEdit->clear();
	Print(crawled->name_);
	Print(str);
	_cacheCsvContent = str;
	//exportExecl(crawled->name_, str);
}

void Windows::CleanWorkSpace() {
	snapShots_.clear();
	firstDiffPage_ = nullptr;
}