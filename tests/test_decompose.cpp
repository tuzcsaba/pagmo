/*****************************************************************************
 *   Copyright (C) 2004-2013 The PaGMO development team,                     *
 *   Advanced Concepts Team (ACT), European Space Agency (ESA)               *
 *   http://apps.sourceforge.net/mediawiki/pagmo                             *
 *   http://apps.sourceforge.net/mediawiki/pagmo/index.php?title=Developers  *
 *   http://apps.sourceforge.net/mediawiki/pagmo/index.php?title=Credits     *
 *   act@esa.int                                                             *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 2 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.               *
 *****************************************************************************/

// Test code for the Decompose(WEIGHTED)

#include <iostream>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
#include <functional>
#include <boost/random/uniform_real.hpp>
#include <cassert>
#include "../src/pagmo.h"
#include "../src/rng.h"
#include "test.h"

using namespace pagmo;

//Generate random number between 0 and 1
double fRand(rng_double drng)
{
    return boost::uniform_real<double>(0,1)(drng);
}

//Generate a random weights vector whose sum of all elements is equal to 1
void generate_weights(fitness_vector & weights)
{
	
	rng_double drng = rng_generator::get<rng_double>();
	
	//generate n random numbers between 0 and 1 and store in weights where n = weights.size()
	std::generate(weights.begin(), weights.end(), std::bind(fRand, drng));
	
	//caculate the sum of all elements of weights vector
	double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
	
	//divide each element of weights vector by the calculated sum to make the sum of all elements equal to 1
	std::transform(weights.begin(), weights.end(), weights.begin(),
	    std::bind1st(std::multiplies<double>(),1/sum));

}

//Construct a new test point d_from_center away from the mid point of upper bound and lower bound for each dimension
decision_vector construct_test_point(const problem::base_ptr &prob, double d_from_center)
{

	assert(abs(d_from_center) <= 1.0);
	decision_vector lb = prob->get_lb();
	decision_vector ub = prob->get_ub();
	decision_vector test_point(prob->get_dimension(), 0);
	for(unsigned int i = 0; i < prob->get_dimension(); i++){
		if(is_eq(ub[i], lb[i])) continue;
		double middle_t = (ub[i] + lb[i]) / 2.0;
		test_point[i] = middle_t + d_from_center * (ub[i] - middle_t);
	}
	return test_point;

}

//Test decompose when weights are not given i.e they are randomly generated by decompose
int test_decompose_weighted_random(const std::vector<problem::base_ptr> &probs, double d_from_center)
{
	
	for(unsigned int i=0; i<probs.size(); i++)
	{
		decision_vector x = construct_test_point(probs[i], d_from_center);
		
		//get the original fitness
		fitness_vector f_original = probs[i]->objfun(x);

		problem::decompose prob_decompose(*(probs[i]), problem::decompose::WEIGHTED);

		//get the decomposed fitness
		fitness_vector f_decompose = prob_decompose.objfun(x);
		
		//get randomly generated weights from decompose
		fitness_vector weights = prob_decompose.get_weights();

		//calculate the expected fitness
		double f_expected = std::inner_product(f_original.begin(), f_original.end(), weights.begin(), 0.0);

		if(is_eq(f_decompose[0], f_expected))
			std::cout<<prob_decompose.get_name()<<" random weights fitness passes, "<<std::endl;
		else
		{
			std::cout<<prob_decompose.get_name()<<" random weights fitness failed, "<<"\t";
			std::cout<<f_expected<<"!="<<f_decompose[0]<<std::endl;
			return 1;
		}
	}
	
	return 0;

}

//Test decompose when weights are given.
int test_decompose_weighted(const std::vector<problem::base_ptr> &probs, double d_from_center)
{
	
	for(unsigned int i=0; i<probs.size(); i++)
	{
		decision_vector x = construct_test_point(probs[i], d_from_center);
		
		fitness_vector weights(probs[i]->get_f_dimension(), 0);
		
		//generate random weights vector
		generate_weights(weights);
		
		//get the original fitness
		fitness_vector f_original = probs[i]->objfun(x);
		
		//calculate expected fitness
		double f_expected = std::inner_product(f_original.begin(), f_original.end(), weights.begin(), 0.0);

		problem::decompose prob_decompose(*(probs[i]), problem::decompose::WEIGHTED, weights);
		
		//get the decomposed fitness
		fitness_vector f_decompose = prob_decompose.objfun(x);

		if(is_eq_vector(weights, prob_decompose.get_weights()))
			std::cout<<prob_decompose.get_name()<<" weights passes, "<<std::endl;
		else
		{
			std::cout<<prob_decompose.get_name()<<" weights failed, "<<std::endl;
			PRINT_VEC(weights);
			PRINT_VEC(prob_decompose.get_weights());
			return 1;
		}

		if(is_eq(f_decompose[0], f_expected))
			std::cout<<prob_decompose.get_name()<<" fitness passes, "<<std::endl;
		else
		{
			std::cout<<prob_decompose.get_name()<<" fitness failed, "<<"\t";
			std::cout<<f_expected<<"!="<<f_decompose[0]<<std::endl;
			return 1;
		}
	}
	
	return 0;

}

int main()
{
	
	int dimension = 40;
	
	std::vector<problem::base_ptr> probs;
	
	for (int i = 1;i <= 6; i++) {
	    probs.push_back(problem::zdt(i,dimension).clone());
	}
	
	for (int i = 1;i <= 7; i++) {
	    probs.push_back(problem::dtlz(i,dimension).clone());
	}

	return test_decompose_weighted(probs, -0.2) ||
		test_decompose_weighted(probs, -0.4) ||
		test_decompose_weighted(probs, 0.2) ||
		test_decompose_weighted_random(probs, -0.4) ||
		test_decompose_weighted_random(probs, 0.2) ||
		test_decompose_weighted_random(probs, -0.2);
}
