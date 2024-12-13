#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

using namespace std;

// Load data from CSV file
vector<vector<double>> loadRatingsMatrix(const string& filepath) {
    vector<vector<double>> matrix;
    ifstream fileStream(filepath);
    string line;

    while (getline(fileStream, line)) {
        vector<double> row;
        string value = "";
        for (char ch : line) {
            if (ch == ',') {
                row.push_back(stod(value));
                value = "";
            } else {
                value += ch;
            }
        }
        row.push_back(stod(value));
        matrix.push_back(row);
    }
    return matrix;
}

// Compute adjusted cosine similarity between two users
double calculateAdjustedSimilarity(const vector<double>& userX, const vector<double>& userY) {
    double sumNumerator = 0, sumDenominatorX = 0, sumDenominatorY = 0;
    double meanX = 0, meanY = 0;
    int count = 0;

    for (size_t i = 0; i < userX.size(); ++i) {
        if (userX[i] > 0 && userY[i] > 0) {
            meanX += userX[i];
            meanY += userY[i];
            count++;
        }
    }
    meanX = (count > 0) ? meanX / count : 0;
    meanY = (count > 0) ? meanY / count : 0;

    for (size_t i = 0; i < userX.size(); ++i) {
        if (userX[i] > 0 && userY[i] > 0) {
            double adjustedX = userX[i] - meanX;
            double adjustedY = userY[i] - meanY;
            sumNumerator += adjustedX * adjustedY;
            sumDenominatorX += adjustedX * adjustedX;
            sumDenominatorY += adjustedY * adjustedY;
        }
    }
    return (sumDenominatorX > 0 && sumDenominatorY > 0) ? sumNumerator / (sqrt(sumDenominatorX) * sqrt(sumDenominatorY)) : 0;
}

// Predict ratings for a user
vector<double> generatePredictions(const vector<vector<double>>& ratings, int userIndex) {
    size_t movieCount = ratings[0].size();
    vector<double> predictedRatings(movieCount, 0);
    vector<double> similarityScores(ratings.size(), 0);

    for (size_t i = 0; i < ratings.size(); ++i) {
        if ((int)i != userIndex) {
            similarityScores[i] = calculateAdjustedSimilarity(ratings[userIndex], ratings[i]);
        }
    }

    for (size_t movie = 0; movie < movieCount; ++movie) {
        if (ratings[userIndex][movie] == 0) {
            double weightedRatingsSum = 0, similarityTotal = 0;
            for (size_t otherUser = 0; otherUser < ratings.size(); ++otherUser) {
                if ((int)otherUser != userIndex && ratings[otherUser][movie] > 0) {
                    weightedRatingsSum += similarityScores[otherUser] * ratings[otherUser][movie];
                    similarityTotal += fabs(similarityScores[otherUser]);
                }
            }
            predictedRatings[movie] = (similarityTotal > 0) ? weightedRatingsSum / similarityTotal : 0;
        }
    }
    return predictedRatings;
}

// Recommend top N movies
vector<int> getTopRecommendations(const vector<double>& predictions, int count) {
    vector<pair<double, int>> movieScores;
    for (size_t i = 0; i < predictions.size(); ++i) {
        movieScores.push_back({predictions[i], (int)i});
    }
    sort(movieScores.rbegin(), movieScores.rend());

    vector<int> topMovies;
    for (int i = 0; i < count && i < (int)movieScores.size(); ++i) {
        if (movieScores[i].first > 0) {
            topMovies.push_back(movieScores[i].second);
        }
    }
    return topMovies;
}

int main() {
    string filePath = "ratings_data.csv";
    vector<vector<double>> userRatings = loadRatingsMatrix(filePath);

    int targetUser = 0; // Target user index for recommendations
    int numberOfRecommendations = 5; // Number of recommendations required

    vector<double> predictedRatings = generatePredictions(userRatings, targetUser);
    vector<int> recommendations = getTopRecommendations(predictedRatings, numberOfRecommendations);

    cout << "Top " << numberOfRecommendations << " movie recommendations for User " << targetUser + 1 << ":\n";
    for (int movieId : recommendations) {
        cout << "Movie " << movieId + 1 << " with predicted rating " << predictedRatings[movieId] << endl;
    }

    return 0;
}
