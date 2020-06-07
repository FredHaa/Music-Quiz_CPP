#include "QuizCreator.hpp"

#include <QList>
#include <QLabel>
#include <QString>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopWidget>
#include <QTableWidgetItem>
#include <QAbstractItemView>

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "common/Log.hpp"
#include "common/TimeUtil.hpp"
#include "util/QuizSettings.hpp"
#include "gui_tools/widgets/QuizTeam.hpp"
#include "gui_tools/widgets/QuizBoard.hpp"
#include "gui_tools/widgets/QuizFactory.hpp"
#include "gui_tools/QuizCreator/EntryCreator.hpp"
#include "gui_tools/QuizCreator/LoadQuizDialog.hpp"
#include "gui_tools/QuizCreator/CategoryCreator.hpp"


MusicQuiz::QuizCreator::QuizCreator(QWidget* parent) :
	QDialog(parent)
{
	/** Set Window Flags */
	setWindowFlags(windowFlags() | Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);

	/** Set Window Title */
	setWindowTitle("Quiz Creator");

	/** Set Object Name */
	setObjectName("QuizCreator");

	/** Set Size */
	const size_t width = 1000;
	const size_t height = 800;
	if ( parent == nullptr ) {
		resize(width, height);
	} else {
		setGeometry(parent->x() + parent->width() / 2 - width / 2, parent->y() + parent->height() / 2 - height / 2, width, height);
	}

	/** Create Audio Player */
	_audioPlayer = std::make_shared<media::AudioPlayer>();

	/** Create Video Player */
	_videoPlayer = std::make_shared<media::VideoPlayer>();	
	_videoPlayer->setWindowFlags(windowFlags() | Qt::Window | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowStaysOnTopHint);

	/** Set Video Player Size */
	const QRect screenRec = QApplication::desktop()->screenGeometry();
	_videoPlayer->setMinimumSize(QSize(screenRec.width() / 4, screenRec.height() / 4));
	_videoPlayer->resize(QSize(screenRec.width() / 4, screenRec.height() / 4));

	/** Create Layout */
	createLayout();
}

void MusicQuiz::QuizCreator::createLayout()
{
	/** Layout */
	QGridLayout* mainlayout = new QGridLayout;

	/** Tab Widget */
	_tabWidget = new QTabWidget;
	mainlayout->addWidget(_tabWidget, 0, 0, 1, 4);

	/** Setup Tab */
	QWidget* setupTab = new QWidget;
	QGridLayout* setupTabLayout = new QGridLayout;
	setupTab->setLayout(setupTabLayout);
	_tabWidget->addTab(setupTab, "Setup");
	size_t row = 0;

	/** Setup Tab - Quiz Name */
	QLabel* label = new QLabel("Quiz Name:");
	label->setObjectName("quizCreatorLabel");
	setupTabLayout->addWidget(label, ++row, 0, 1, 2, Qt::AlignLeft);

	_quizNameLineEdit = new QLineEdit;
	_quizNameLineEdit->setObjectName("quizCreatorLineEdit");
	setupTabLayout->addWidget(_quizNameLineEdit, ++row, 0, 1, 2);

	/** Setup Tab - Quiz Description */
	label = new QLabel("Quiz Description:");
	label->setObjectName("quizCreatorLabel");
	setupTabLayout->addWidget(label, ++row, 0, 1, 2, Qt::AlignLeft);

	_quizDescriptionTextEdit = new QTextEdit;
	_quizDescriptionTextEdit->setObjectName("quizCreatorTextEdit");
	setupTabLayout->addWidget(_quizDescriptionTextEdit, ++row, 0, 1, 2);

	/** Setup Tab - Settings */
	label = new QLabel("Settings:");
	label->setObjectName("quizCreatorLabel");
	setupTabLayout->addWidget(label, ++row, 0, 1, 2, Qt::AlignLeft);

	_hiddenCategoriesCheckbox = new QCheckBox("Hidden Categories");
	_hiddenCategoriesCheckbox->setObjectName("quizCreatorCheckbox");
	setupTabLayout->addWidget(_hiddenCategoriesCheckbox, ++row, 0, 1, 2);

	/** Setup Tab - Categories */
	label = new QLabel("Categories:");
	label->setObjectName("quizCreatorLabel");
	setupTabLayout->addWidget(label, ++row, 0, 1, 1, Qt::AlignLeft);

	QPushButton* addCategoryBtn = new QPushButton;
	addCategoryBtn->setObjectName("quizCreatorAddBtn");
	addCategoryBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	connect(addCategoryBtn, SIGNAL(released()), this, SLOT(addCategory()));
	setupTabLayout->addWidget(addCategoryBtn, row, 1, 1, 1, Qt::AlignRight);

	_categoriesTable = new QTableWidget(0, 3, this);
	_categoriesTable->setObjectName("quizCreatorTable");
	_categoriesTable->setDragDropMode(QAbstractItemView::InternalMove);
	_categoriesTable->setSelectionMode(QAbstractItemView::SingleSelection);
	_categoriesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	_categoriesTable->setStyleSheet("QHeaderView { qproperty-defaultAlignment: AlignCenter; }");
	_categoriesTable->horizontalHeader()->setVisible(false);
	_categoriesTable->horizontalHeader()->setMinimumWidth(40);
	_categoriesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	_categoriesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	_categoriesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	_categoriesTable->verticalHeader()->setFixedWidth(40);
	_categoriesTable->verticalHeader()->setSectionsMovable(false); // \todo set this to true to enable dragging.
	_categoriesTable->verticalHeader()->setDefaultSectionSize(40);
	_categoriesTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed); 
	connect(_categoriesTable->verticalHeader(), SIGNAL(sectionMoved(int, int, int)), this, SLOT(categoryOrderChanged(int, int, int)));
	setupTabLayout->addWidget(_categoriesTable, ++row, 0, 1, 2);

	/** Setup Tab - Row Categories */
	label = new QLabel("Row Categories:");
	label->setObjectName("quizCreatorLabel");
	setupTabLayout->addWidget(label, ++row, 0, 1, 2, Qt::AlignLeft);

	QPushButton* addRowCategoryBtn = new QPushButton;
	addRowCategoryBtn->setObjectName("quizCreatorAddBtn");
	addRowCategoryBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	connect(addRowCategoryBtn, SIGNAL(released()), this, SLOT(addRowCategory()));
	setupTabLayout->addWidget(addRowCategoryBtn, row, 1, 1, 1, Qt::AlignRight);

	_rowCategoriesTable = new QTableWidget(0, 2);
	_rowCategoriesTable->setObjectName("quizCreatorTable");
	_rowCategoriesTable->horizontalHeader()->setVisible(false);
	_rowCategoriesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	_rowCategoriesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	_rowCategoriesTable->verticalHeader()->setDefaultSectionSize(40);
	setupTabLayout->addWidget(_rowCategoriesTable, ++row, 0, 1, 2);

	/** Bottom Buttons */
	QPushButton* previewQuizBtn = new QPushButton("Preview");
	previewQuizBtn->setObjectName("quizCreatorBtn");
	connect(previewQuizBtn, SIGNAL(released()), this, SLOT(previewQuiz()));
	mainlayout->addWidget(previewQuizBtn, 1, 0, 1, 1);

	QPushButton* saveQuizBtn = new QPushButton("Save Quiz");
	saveQuizBtn->setObjectName("quizCreatorBtn");
	connect(saveQuizBtn, SIGNAL(released()), this, SLOT(saveQuiz()));
	mainlayout->addWidget(saveQuizBtn, 1, 1, 1, 1);

	QPushButton* loadQuizBtn = new QPushButton("Load Quiz");
	loadQuizBtn->setObjectName("quizCreatorBtn");
	connect(loadQuizBtn, SIGNAL(released()), this, SLOT(openLoadQuizDialog()));
	mainlayout->addWidget(loadQuizBtn, 1, 2, 1, 1);

	QPushButton* quitCreatorBtn = new QPushButton("Quit");
	quitCreatorBtn->setObjectName("quizCreatorBtn");
	connect(quitCreatorBtn, SIGNAL(released()), this, SLOT(quitCreator()));
	mainlayout->addWidget(quitCreatorBtn, 1, 3, 1, 1);

	/** Set Layout */
	setLayout(mainlayout);
}

void MusicQuiz::QuizCreator::addCategory()
{
	/** Sanity Check */
	if ( _categoriesTable == nullptr ) {
		return;
	}

	/** Get Number of Categories */
	const size_t categoryCount = _categoriesTable->rowCount();

	/** Insert New Category */
	_categoriesTable->insertRow(categoryCount);

	/** Add Line Edit */
	const QString categoryNameStr = "Category " + QString::number(categoryCount + 1);
	QLineEdit* categoryName = new QLineEdit(categoryNameStr);
	categoryName->setObjectName("quizCreatorCategoryLineEdit");
	categoryName->setProperty("index", categoryCount);
	connect(categoryName, SIGNAL(textChanged(const QString&)), this, SLOT(updateCategoryTabName(const QString&)));
	_categoriesTable->setCellWidget(categoryCount, 0, categoryName);

	/** Add Edit Category Button */
	QPushButton* editBtn = new QPushButton;
	editBtn->setObjectName("quizCreatorEditBtn");
	editBtn->setProperty("index", categoryCount);
	connect(editBtn, SIGNAL(released()), this, SLOT(editCategory()));

	QHBoxLayout* btnLayout = new QHBoxLayout;
	btnLayout->addWidget(editBtn, Qt::AlignCenter);

	QWidget* layoutWidget = new QWidget;
	layoutWidget->setLayout(btnLayout);
	_categoriesTable->setCellWidget(categoryCount, 1, layoutWidget);

	/** Add Remove Category Button */
	QPushButton* removeBtn = new QPushButton;
	removeBtn->setObjectName("quizCreatorRemoveBtn");
	removeBtn->setProperty("index", categoryCount);
	connect(removeBtn, SIGNAL(released()), this, SLOT(removeCategory()));

	btnLayout = new QHBoxLayout;
	btnLayout->addWidget(removeBtn, Qt::AlignCenter);

	layoutWidget = new QWidget;
	layoutWidget->setLayout(btnLayout);
	_categoriesTable->setCellWidget(categoryCount, 2, layoutWidget);

	/** Add Tab */
	MusicQuiz::CategoryCreator* category = new MusicQuiz::CategoryCreator(categoryNameStr, _audioPlayer);
	_categories.push_back(category);
	_tabWidget->addTab(category, categoryNameStr);
}

void MusicQuiz::QuizCreator::addRowCategory()
{
	/** Sanity Check */
	if ( _rowCategoriesTable == nullptr ) {
		return;
	}

	/** Get Number of Categories */
	const size_t rowCategoryCount = _rowCategoriesTable->rowCount();

	/** Insert New Category */
	_rowCategoriesTable->insertRow(rowCategoryCount);

	/** Add Line Edit */
	QLineEdit* rowCategoryName = new QLineEdit("Row Category " + QString::number(rowCategoryCount + 1));
	rowCategoryName->setObjectName("quizCreatorCategoryLineEdit");
	_rowCategoriesTable->setCellWidget(rowCategoryCount, 0, rowCategoryName);

	/** Add Remove Category Button */
	QPushButton* removeBtn = new QPushButton("X");
	removeBtn->setObjectName("quizCreatorRemoveBtn");
	removeBtn->setProperty("index", rowCategoryCount);
	connect(removeBtn, SIGNAL(released()), this, SLOT(removeRowCategory()));

	QHBoxLayout* removeBtnLayout = new QHBoxLayout;
	removeBtnLayout->addWidget(removeBtn, Qt::AlignCenter);

	QWidget* layoutWidget = new QWidget;
	layoutWidget->setLayout(removeBtnLayout);
	_rowCategories.push_back(rowCategoryName->text());
	_rowCategoriesTable->setCellWidget(rowCategoryCount, 1, layoutWidget);
}

void MusicQuiz::QuizCreator::editCategory()
{
	/** Sanity Check */
	if ( _categoriesTable == nullptr ) {
		return;
	}

	QPushButton* button = qobject_cast<QPushButton*>(sender());
	if ( button == nullptr ) {
		return;
	}

	/** Get Number of Categories */
	const size_t categoryCount = _categoriesTable->rowCount();

	/** Get Index */
	const size_t index = button->property("index").toInt();
	if ( index >= _tabWidget->count() || index >= categoryCount ) {
		return;
	}	
	
	/** Go to the Tab */
	_tabWidget->setCurrentIndex(index + 1);
}

void MusicQuiz::QuizCreator::removeCategory()
{
	/** Sanity Check */
	if ( _categoriesTable == nullptr ) {
		return;
	}

	QPushButton* button = qobject_cast<QPushButton*>(sender());
	if ( button == nullptr ) {
		return;
	}

	/** Get Number of Categories */
	const size_t categoryCount = _categoriesTable->rowCount();

	/** Get Index */
	const size_t index = button->property("index").toInt();
	if ( index >= _tabWidget->count() || index >= categoryCount ) {
		return;
	}

	/** Get Category Name */
	QLineEdit* lineEdit = qobject_cast<QLineEdit*>(_categoriesTable->cellWidget(index, 0));
	if ( lineEdit == nullptr ) {
		return;
	}

	/** Popup to ensure the user wants to delete the category */
	QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Delete Category?", "Are you sure you want to delete category '" + lineEdit->text() + "'?",
		QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
	if ( resBtn != QMessageBox::Yes ) {
		return;
	}

	/** Delete Category in table */
	_categoriesTable->removeRow(index);
	_categories.erase(_categories.begin() + index);

	/** Delete Tab */
	_tabWidget->removeTab(index + 1);

	/** Update Indices */
	for ( size_t i = 0; i < _categoriesTable->rowCount(); ++i ) {
		/** Line Edit */
		QLineEdit* tmpLineEdit = qobject_cast<QLineEdit*>(_categoriesTable->cellWidget(i, 0));
		if ( tmpLineEdit != nullptr ) {
			tmpLineEdit->setProperty("index", i);
		}

		/** Button */
		QList<QPushButton*> buttons = _categoriesTable->cellWidget(i, 1)->findChildren<QPushButton*>() + _categoriesTable->cellWidget(i, 2)->findChildren<QPushButton*>();
		for ( QPushButton* tmpButton : buttons ) {
			if ( tmpButton != nullptr ) {
				if ( tmpButton->property("index").toInt() == index ) {
					tmpButton = nullptr;
					delete tmpButton;
				} else {
					tmpButton->setProperty("index", i);
				}
			}
		}
	}
}

void MusicQuiz::QuizCreator::removeRowCategory()
{
	/** Sanity Check */
	if ( _rowCategoriesTable == nullptr ) {
		return;
	}

	QPushButton* button = qobject_cast<QPushButton*>(sender());
	if ( button == nullptr ) {
		return;
	}

	/** Get Number of Row Categories */
	const size_t rowCategoryCount = _rowCategoriesTable->rowCount();

	/** Get Index */
	const size_t index = button->property("index").toInt();
	if ( index >= rowCategoryCount ) {
		return;
	}

	/** Get Row Category Name */
	QLineEdit* lineEdit = qobject_cast<QLineEdit*>(_rowCategoriesTable->cellWidget(index, 0));
	if ( lineEdit == nullptr ) {
		return;
	}
	const QString rowCategoryName = lineEdit->text();

	/** Popup to ensure the user wants to delete the row category */
	QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Delete Row Category?", "Are you sure you want to delete row category '" + rowCategoryName + "'?",
		QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
	if ( resBtn != QMessageBox::Yes ) {
		return;
	}

	/** Delete Row Category in table */
	_rowCategoriesTable->removeRow(index);
	_rowCategories.erase(_rowCategories.begin() + index);

	/** Update Button Indices */
	for ( size_t i = 0; i < _rowCategoriesTable->rowCount(); ++i ) {
		QList<QPushButton*> buttons = _rowCategoriesTable->cellWidget(i, 1)->findChildren<QPushButton*>();
		for ( QPushButton* tmpButton : buttons ) {
			if ( tmpButton != nullptr ) {
				tmpButton->setProperty("index", i);
			}
		}
	}
}

void MusicQuiz::QuizCreator::updateCategoryTabName(const QString& str)
{
	/** Sanity Check */
	if ( _tabWidget == nullptr ) {
		return;
	}

	QLineEdit* lineEdit = qobject_cast<QLineEdit*>(sender());
	if ( lineEdit == nullptr ) {
		return;
	}

	/** Get Index */
	const size_t index = lineEdit->property("index").toInt() + 1;
	if ( index >= _tabWidget->count() ) {
		return;
	}

	/** Set Tab Text */
	_tabWidget->setTabText(index, lineEdit->text());

	/** Update Category Name */
	MusicQuiz::CategoryCreator* categoryWidget = qobject_cast<MusicQuiz::CategoryCreator*>(_tabWidget->widget(index));
	if ( categoryWidget != nullptr ) {
		categoryWidget->setName(str);
	}
}

void MusicQuiz::QuizCreator::saveQuiz()
{
	/** Stop Song */
	_audioPlayer->stop();

	/** Get Quiz Data */
	QuizData quizData;

	/** Quiz Name */
	quizData.quizName = _quizNameLineEdit->text();

	/** Quiz Author */
	quizData.quizAuthor = ""; // \todo add this.

	/** Quiz Description */
	quizData.quizDescription = _quizDescriptionTextEdit->toPlainText();

	/** Hidden Categoies */
	quizData.guessTheCategory = _hiddenCategoriesCheckbox->isChecked();
	quizData.guessTheCategoryPoints = 500; // \todo implement this.

	/** Quiz Categories */
	quizData.quizCategories = _categories;

	/** Quiz Row Categories */
	quizData.quizRowCategories = _rowCategories;

	/** Save Quiz */
	MusicQuiz::QuizFactory::saveQuiz(quizData, this);
}

void MusicQuiz::QuizCreator::openLoadQuizDialog()
{
	/** Create Dialog */
	MusicQuiz::LoadQuizDialog* loadQuizDialog = new MusicQuiz::LoadQuizDialog(this);

	/** Connect Signal */
	connect(loadQuizDialog, SIGNAL(loadSignal(const std::string&)), this, SLOT(loadQuiz(const std::string&)));

	/** Open Dialog */
	loadQuizDialog->exec();
}

void MusicQuiz::QuizCreator::loadQuiz(const std::string& quizName)
{
	/** Sanity Check */
	if ( quizName.empty() ) {
		return;
	}

	LOG_INFO("Loading quiz '" << quizName << "'");
}

void MusicQuiz::QuizCreator::previewQuiz()
{
	/** Stop Song */
	_audioPlayer->stop();
	_videoPlayer->stop();

	/** Check that quiz have been saved */
	const std::string quizName = _quizNameLineEdit->text().toStdString();
	const std::string quizPath = "./data/" + quizName + "/" + quizName + ".quiz.xml", tree;
	if ( !boost::filesystem::exists(quizPath) ) {
		QMessageBox::information(nullptr, "Info", "Quiz must be saved before the preview can be shown.");
		return;
	}

	/** Save Quiz */
	try {
		//saveQuiz();
	} catch ( ... ) {}

	/** Create Dummy Teams */
	MusicQuiz::QuizTeam* dummyTeamOne = new QuizTeam("Team 1", QColor(255, 0, 0));
	MusicQuiz::QuizTeam* dummyTeamTwo = new QuizTeam("Team 2", QColor(0, 255, 0));
	const std::vector< MusicQuiz::QuizTeam* > dummyTeams = { dummyTeamOne, dummyTeamTwo };

	/** Dummy Settings */
	MusicQuiz::QuizSettings settings;
	settings.guessTheCategory = _hiddenCategoriesCheckbox->isChecked();

	/** Create Quiz Preview */
	try {
		_previewQuizBoard = MusicQuiz::QuizFactory::createQuiz(quizPath, settings, _audioPlayer, _videoPlayer, dummyTeams, true, this);
		if ( _previewQuizBoard == nullptr ) {
			QMessageBox::warning(this, "Info", "Failed to preview quiz.");
			return;
		}

		/** Connect Signals */
		connect(_previewQuizBoard, SIGNAL(quitSignal()), this, SLOT(stopQuizPreview()));

		/** Start Preview */
		_previewQuizBoard->show();
	} catch ( const std::exception& err ) {
		QMessageBox::warning(this, "Info", "Failed to preview quiz. " + QString::fromStdString(err.what()));
		return;
	} catch ( ... ) {
		QMessageBox::warning(this, "Info", "Failed to preview quiz.");
		return;
	}
}

void MusicQuiz::QuizCreator::stopQuizPreview()
{
	/** Sanity Check */
	if ( _previewQuizBoard == nullptr ) {
		return;
	}

	/** Stop Quiz */
	_previewQuizBoard->hide();
	_previewQuizBoard->close();
	delete _previewQuizBoard;
	_previewQuizBoard = nullptr;
}

void MusicQuiz::QuizCreator::quitCreator()
{
	QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Close Quiz Creator?", "Are you sure you want to close the Quiz Creator?",
		QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);

	if ( resBtn == QMessageBox::Yes ) {
		/** Call Destructor */
		QApplication::quit();
	}
}

void MusicQuiz::QuizCreator::categoryOrderChanged(const int, const int oldIdx, const int newIdx)
{
	/** Sanity Check */
	if ( _categoriesTable == nullptr || _tabWidget == nullptr ) {
		return;
	}

	// \todo write the code for chaning the order of the tabs and the vector.
}