/*
 *   Improved Genetic Algorithm for TSP Problem.
 *           --- Li Yunze 2022301052141
 */

// ** HEADER ****************************************************************************** //

#include <map>
#include <cmath>
#include <ctime>
#include <tuple>
#include <vector>
#include <random>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

// ** TOOL ******************************************************************************** //

namespace TOOL {

    double  // Return a random double from [minV, maxV]
    randDou(
        double minV = 0, 
        double maxV = 1
    ) {
        return minV + (static_cast<double>(rand()) / RAND_MAX) * (maxV - minV);
    }

    int     // Return a random int from [minV, maxV]
    randInt(    // WARNING: Numbers generated by rand() range from 0 to 32767
        int minV = 0,
        int maxV = 1
    ) {
        return minV + rand() % (maxV - minV + 1);
    }

    bool    // Decide whether a probability can be satisfied
    confirm(
        double probability
    ) {
        double get = randDou();
        return (0.0 <= get) && (get <= probability);
    }

    void    // Output a solution
    outputSeq(
        const std::vector<int> & sol
    ) {
        for (int i = 0; i < sol.size(); i++) {
            printf("%02d -> ", sol[i]);
        }
        printf("%02d ", sol[0]);
    }
    
}

// ** VARIABLE **************************************************************************** //

const int maxCity = 1000;           // Max number of cities
double dis[maxCity][maxCity];       // Distance between cities
int cityNum;                        // Number of cities

const double proCross  = 0.88;      // Probability of CROSSOVER
const double proMutate = 0.07;      // Probability of MUTATE
const double proSelect = 0.40;      // Probability of SELECT individuals via ROULETTE

const int maxEndure       = 5;      // Max times before stoping finding a position in CROSSOVER
const int newIndividual   = 150;    // Number of new individuals according to population update strategy based on group similarity
const double maxDeviation = 5e6;    // Max Deviation can be accepted in population update strategy

const int generationNum   = 1000;   // Number of generations
const int populationSize  = 300;    // Size of population

// ** FUNCTION **************************************************************************** //

bool    // Input map information and check success
InputInfo() {
    
    std::fstream file("map.txt");

    if (file.is_open()) {

        file >> cityNum;
        for (int i = 1; i <= cityNum; i++) {
            for (int j = 1; j <= cityNum; j++) {
                file >> dis[i][j];
            }
        }
        
        return true;

    } else return false;
    
}

void    // Initialize the population
Initialize(
    std::vector<std::vector<int>> & popu
) {

    // Decide seed according to time
    srand(time(NULL) + 114514);
    std::mt19937 randAlgo(time(NULL) + 1919810);

    // Decide a random arrangement for every population
    for (auto & item: popu) {
        item.resize(cityNum);
        for (int i = 0; i < cityNum; i++) {
            item[i] = i + 1;
        }
        std::shuffle(item.begin(), item.end(), randAlgo);
    }

}

double  // Target function
Target(
    const std::vector<int> & sol
) {

    double sumSol = dis[sol[cityNum - 1]][sol[0]];  // Sum distance of a solution
    for (int i = 0; i < cityNum - 1; i++) {
        sumSol += dis[sol[i]][sol[i + 1]];
    }

    return sumSol;

}

double  // Calculate the fitness
Fitness(
    const std::vector<int> & sol  // Solution
) {

    double sumSol = Target(sol);
    return 1e7 / sumSol;    // Make better solution's fitness bigger
    
}

void    // Roulette and Fitness for choose individuals
Select(
    std::vector<double> fit,
    std::vector<int> & answer,
    int count   //Number of chosen individuals
) {

    int sizeFit = fit.size();
    std::vector<double> roulette(sizeFit + 1);  // Roulette
    std::vector<double> rate(sizeFit);          // All results (count times) of roulette
    answer.resize(sizeFit); 

    double sumFit = [&] {
        double ans = 0;
        for (auto item: fit) ans += item;
        return ans;
    } ();

    // Calculate the roulette (fit[i] / sum(fit[i]))
    for (int i = 0; i < sizeFit; i++) {
        roulette[i + 1] = roulette[i] + fit[i] / sumFit;
    }

    // Get every probability (count times) of roulette
    for (int i = 0; i < count; i++) {
        rate[i] = TOOL::randDou();
    }
    std::sort(rate.begin(), rate.end());

    // Decide every result of roulette according to rate (probability)
    for (int i = 0, now = 0; i < sizeFit && now < count; i++) {
        while (roulette[i] <= rate[now] && rate[now] < roulette[i + 1]) {
            answer[now++] = i;
            if (now >= count) break;
        }
    }

    // Choose the remaining individuals according to fitness
    std::vector<int> index(sizeFit);
    std::iota(index.begin(), index.end(), 0);   // Initialize index begin with 0 (0, 1, ..., sizeFit - 1)
    std::sort(index.begin(), index.end(), [&] (const int & a, const int & b) {
        return fit[a] > fit[b];
    });
    
    for (int i = count; i < sizeFit; i++) {
        answer[i] = index[i - count];
    }

    // Shuffle the answer
    std::mt19937 randAlgo(time(NULL) + rand());
    std::shuffle(answer.begin(), answer.end(), randAlgo);

}

int     // Return the NextPos of nowPos
NextPos(
    int nowPos
) {
    return (nowPos + 1) % cityNum;
}

int     // Find the position of value (target) from solution
FindPos(
    const std::vector<int> & sol,
    int target
) {

    for (int i = 0; i < sol.size(); i++) {
        if (sol[i] == target) return i;
    }
    
    return -1;

}

std::vector<int>    // Generate offspring ORDEREDLY based on a new crossover operator
CrossoverOrdered(
    const std::vector<int> & father,    // Duty of father and mother and different
    const std::vector<int> & mother
) {

    std::vector<int> ans(cityNum);
    for (int i = 0; i < cityNum; i++) {
        ans[i] = father[i];
    }

    int fatPos, motPos, count = 0;
    int std = TOOL::randInt(1, cityNum);
    while (
        dis[std][father[(fatPos = NextPos(FindPos(father, std)))]] <= 
        dis[std][mother[(motPos = NextPos(FindPos(mother, std)))]]
    ) {
        std = father[fatPos];
        if (++count >= maxEndure) break;    // The maximum number of attempts before finding a position
    }

    std::swap(ans[fatPos], ans[FindPos(father, mother[motPos])]);

    return ans;

}

std::tuple<std::vector<int>, std::vector<int>>  // Crossover two parents and generate offspring
Crossover(
    const std::vector<int> & parentA,   // Two Parents
    const std::vector<int> & parentB
) {

    if (TOOL::confirm(proCross)) {  // When random number hit the PROBABILITY
        return std::make_tuple(
            CrossoverOrdered(parentA, parentB),
            CrossoverOrdered(parentB, parentA)
        );
    } else {
        return std::make_tuple(parentA, parentB);
    }
    
}

bool    // Check if the condition of [Population update strategy based on group similarity] is satisfied
UpdateConfirm(
    const std::vector<std::vector<int>> population,
    int popuSize
) {

    std::vector<double> target(popuSize);  // Target of population
    for (int i = 0; i < popuSize; i++) {
        target[i] = Target(population[i]);
    }
    
    double avgTarget = [&] {    // Average Target of each population
        double sum = 0;
        for (int i = 0; i < popuSize; i++) sum += target[i];
        return sum / popuSize;
    } ();

    double sumDeviation = [&] { // Sum of Deviation (sum(|avgTarget - target[i]|))
        double sum = 0;
        for (int i = 0; i < popuSize; i++) sum += fabs(target[i] - avgTarget);
        return sum;
    } ();

    // Check if sum of deviation smaller than the max deviation configuration
    return sumDeviation <= maxDeviation;
    
}

void    // Mutate some solutions
Mutate(
    std::vector<int> & sol
) {

    if (TOOL::confirm(proMutate)) {
        int a = TOOL::randInt(0, cityNum - 1), b; // Generate two mutate points
        while (a == (b = TOOL::randInt(0, cityNum - 1)));
        std::swap(sol[a], sol[b]);
    } else return;
    
}

// ** GENETIC ALGORITHM ******************************************************************* //

void    // Genetic Algorithm 
GeneticAlgorithm(
    int genNum,     // Number of generations
    int popuSize    // Size of populations
) {

    // Initialize
    std::vector<std::vector<int>> population(popuSize * 2);   // Populations
    Initialize(population);

    // Global best solution
    std::pair<std::vector<int>, double> globalBest;
    std::fstream sequence("./result/IGA.seq", std::ios::out | std::ios::trunc);

    for (int gen = 0; gen < genNum; gen++) {
        
        // Calculate the fitness and find the best solution
        std::pair<int, double> best({0, 0});    // best {Index, fitness}
        std::vector<double> fit(popuSize);      // Fitness of population
        for (int i = 0; i < popuSize; i++) {
            fit[i] = Fitness(population[i]);
            if (fit[i] > best.second) best = {i, fit[i]};
        }

        // Output best individual in this round
        printf("[Generation] %04d [Solution] ", gen + 1);
        TOOL::outputSeq(population[best.first]);
        printf("[Target] %.4lf\n", Target(population[best.first]));
        sequence << std::fixed << std::setprecision(4) 
            << Target(population[best.first]) << (gen == (genNum - 1) ? "" : ", ");

        // Update global best solution
        if (best.second > globalBest.second) {
            globalBest.first.resize(cityNum);
            globalBest.second = best.second;
            for (int i = 0; i < cityNum; i++) {
                globalBest.first[i] = population[best.first][i];
            }
        }

        // Select the individuals with ROULETTE
        std::vector<int> chosen;
        Select(fit, chosen, popuSize * proSelect);

        // Crossover to generate offspring according to [Population update strategy based on group similarity]
        if (UpdateConfirm(population, popuSize)) {

            for (int i = 0; i < popuSize && i < newIndividual; i += 2) {
                std::tie(population[i + popuSize], population[i + popuSize + 1]) 
                    = Crossover(population[chosen[i]], population[chosen[i + 1]]);
            }

            std::sort(population.begin(), population.end(), 
                [&] (const std::vector<int> & a, const std::vector<int> & b) {
                    return Fitness(a) > Fitness(b);
                }
            );

        }

        // Mutate some individuals
        for (int i = 0; i < popuSize; i++) {
            Mutate(population[i]);
        }

    }

    // Output global best solution
    printf("\n  --> [Global Best Solution] ");
    TOOL::outputSeq(globalBest.first);
    printf("[Target] %.4lf\n", Target(globalBest.first));

}

// ** MAIN ******************************************************************************** //

int main() {
    freopen("./result/IGA.log", "w", stdout);
    if (InputInfo()) GeneticAlgorithm(generationNum, populationSize);
    else puts("Input information fail.");
}