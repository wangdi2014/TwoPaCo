#ifndef _ASSEMBLY_EDGE_CONSTRUCTOR_H_
#define _ASSEMBLY_EDGE_CONSTRUCTOR_H_

#include "vertexenumerator.h"

namespace TwoPaCo
{
	class AssemblyEdgeConstructor
	{
	public:
		AssemblyEdgeConstructor(const std::vector<std::string> & inputFileName, const std::string & marksFileName, const VertexEnumerator & vertexEnumerator) : 				
				vertexEnumerator_(vertexEnumerator)
		{
			int64_t vertexLength = vertexEnumerator_.GetHashSeed().VertexLength();
			int64_t edgeLength = vertexLength + 1;

			size_t chrNumber = 0;
			ChrReader chrReader(inputFileName);
			std::vector<std::vector<bool> > candidateMarks;
			vertexEnumerator_.ReloadJunctionCandidateMask(candidateMarks);
			std::unique_ptr<ConcurrentBitVector> bloomFilter = vertexEnumerator_.ReloadBloomFilter();
			for (std::string chr; chrReader.NextChr(chr); chrNumber++)
			{												
				//Init hash function				
				VertexRollingHash hash(vertexEnumerator.GetHashSeed(), chr.begin(), vertexEnumerator.GetHashSeed().HashFunctionsNumber());
				for (int64_t i = 0; i <= int64_t(chr.size()) - edgeLength; i++)
				{
					std::string vertex = chr.substr(i, vertexLength);
					//Check if the Bloom filter contains an edge
					assert(IsOutgoingEdgeInBloomFilter(hash, *bloomFilter, chr[i + edgeLength - 1]));
					if (i > 0)
					{
						assert(IsIngoingEdgeInBloomFilter(hash, *bloomFilter, chr[i - 1]));
					}
					
					//Check the if the vertex is a junction candidate
					std::string inEdges;
					std::string outEdges;
					if (vertexEnumerator_.GetEdges(vertex.begin(), hash, inEdges, outEdges))
					{
						//Found a junction candidate, check that the mark in the vector is set
						assert(candidateMarks[chrNumber][i] == true);
					}

					hash.Update(chr[i], chr[i + vertexLength]);
					//Check that the hash values were updated correctly
					assert(hash.Assert(chr.begin() + i + 1));					
				}
			}
		}
		
	private:
		const VertexEnumerator & vertexEnumerator_;
		DISALLOW_COPY_AND_ASSIGN(AssemblyEdgeConstructor);
	};


}

#endif