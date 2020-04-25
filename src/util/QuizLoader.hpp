#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <boost/filesystem.hpp>


namespace MusicQuiz {
	namespace util {
		class QuizLoader
		{
		public:
			struct QuizPreview
			{
				std::string quizName = "";
				bool includeSongs = false;
				bool includeVideos = false;
				std::string quizDescription = "";
				std::vector<std::string> categories;
				std::vector<std::string> rowCategories;

				friend std::ostream& operator<<(std::ostream& out, const QuizPreview& quizPreview)
				{
					out << "\n\nQuiz Name: " << quizPreview.quizName << "\n";
					out << "Quiz Description: " << quizPreview.quizDescription << "\n";
					if ( !quizPreview.categories.empty() ) {
						out << "Quiz Categories:\n";
						for ( size_t i = 0; i < quizPreview.categories.size(); ++i ) {
							out << "\t" << quizPreview.categories[i] << "\n";
						}
					}

					if ( !quizPreview.rowCategories.empty() ) {
						out << "Quiz Row Categories:\n";
						for ( size_t i = 0; i < quizPreview.rowCategories.size(); ++i ) {
							out << "\t" << quizPreview.rowCategories[i] << "\n";
						}
					}

					out << "Quiz Include Songs: " << (quizPreview.includeSongs ? "Yes" : "No") << "\n";
					out << "Quiz Include Videos: " << (quizPreview.includeVideos ? "Yes" : "No") << "\n\n";
					return out;
				}
			};

			/**
			 * @brief Deleted constructor.
			 */
			QuizLoader() = delete;

			/**
			* @brief Deleted Destructor.
			*/
			~QuizLoader() = delete;

			/**
			* @brief Returns a list of quizzez stored in the data folder.
			*
			* @return The list of quizzes.
			*/
			static std::vector<std::string> getListOfQuizzes();

			/**
			* @brief Returns a quiz preview.
			*
			* @param[in] idx The index of the quiz to preview.
			*
			* @return The quiz preview.
			*/
			static QuizPreview getQuizPreview(size_t idx);

		protected:
			/** Variables */
		};


	}
}
