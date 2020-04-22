#include "QuizTeam.hpp"

#include <sstream>
#include <stdexcept>

#include <QMouseEvent>

#include "common/Log.hpp"


MusicQuiz::QuizTeam::QuizTeam(const QString& name, const QColor& color, QWidget* parent) :
	QPushButton(parent), _name(name), _color(color)
{
	/** Sanity Check */
	if ( _name.isEmpty() ) {
		throw std::runtime_error("Cannot create team without a name.");
	}

	/** Set Team Text */
	QString str = _name + ": " + QString(_score);
	setText(str.toLocal8Bit());

	/** Set Background Color */
	std::stringstream ss;
	ss << "background-color	: rgb(" << _color.red() << ", " << _color.green() << ", " << _color.blue() << ");";

	/** Set Text Color to inverted button color */
	ss << "color : rgb(" << 255 - _color.red() << ", " << 255 - _color.green() << ", " << 255 - _color.blue() << ");";
	setStyleSheet(QString::fromStdString(ss.str()));

	/** Set Object Name */
	setObjectName("TeamEntry");
}

void MusicQuiz::QuizTeam::addPoints(size_t points)
{
	/** Update Score */
	_score += points;

	/** Update Text */
	QString str = _name + ": " + QString(_score);
	setText(str.toLocal8Bit());
}

size_t MusicQuiz::QuizTeam::getScore()
{
	return _score;
}