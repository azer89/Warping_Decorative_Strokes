#include "ConformalMapping.h"

#include "AVector.h"
#include "ALine.h"
#include "PlusSignVertex.h"

#include "UtilityFunctions.h"

ConformalMapping::ConformalMapping() :
    _iterDist(std::numeric_limits<float>::max())
{
}

void ConformalMapping::ConformalMappingOneStepSimple(std::vector<QuadMesh>& quadMeshes)
{
    this->_iterDist = 0;
    for(uint a = 0; a < quadMeshes.size(); a++)
    {
        if(quadMeshes[a]._quadMeshType == QuadMeshType::MESH_KITE)
        {
            ConformalMappingOneStepSimple(&quadMeshes[a]);
        }
    }
}

void ConformalMapping::ConformalMappingOneStepSimple(QuadMesh *qMesh)
{
    std::vector<std::vector<PlusSignVertex>> tempVertices = qMesh->_plusSignVertices;
    int meshWidth = qMesh->_plusSignVertices.size();
    int meshHeight = qMesh->_plusSignVertices[0].size();

    for(int a = 0; a < meshWidth; a++)
    {
        for(int b = 0; b < meshHeight; b++)
        {
            if(!tempVertices[a][b].shouldMove)
                { continue; }

            AVector sumPositions(0, 0);
            int numNeighbor = 0;

            // left
            if(a > 0)
            {
                sumPositions += tempVertices[a - 1][b].position;
                numNeighbor++;
            }

            // right
            if(a < meshWidth - 1)
            {
                sumPositions = sumPositions + tempVertices[a + 1][b].position;
                numNeighbor++;
            }

            // up
            if(b > 0)
            {
                sumPositions += tempVertices[a][b - 1].position;
                numNeighbor++;
            }

            // bottom
            if(b < meshHeight - 1)
            {
                sumPositions += tempVertices[a][b + 1].position;
                numNeighbor++;
            }

            sumPositions  = sumPositions / (float)numNeighbor;

            if(numNeighbor < 4)
            {
                tempVertices[a][b].position = GetClosestPointFromBorders(*qMesh, sumPositions);
            }
            else
            {
                tempVertices[a][b].position = sumPositions;
            }
        }
    }

    float sumDist = 0;
    for(int a = 0; a < meshWidth; a++)
    {
        for(int b = 0; b < meshHeight; b++)
        {
            sumDist += qMesh->_plusSignVertices[a][b].position.Distance(tempVertices[a][b].position);
        }
    }
    _iterDist = sumDist;
    qMesh->_plusSignVertices = tempVertices;
}

void ConformalMapping::ConformalMappingOneStep(std::vector<QuadMesh>& quadMeshes)
{
    this->_iterDist = 0;
    for(uint a = 0; a < quadMeshes.size(); a++)
    {
        if(quadMeshes[a]._quadMeshType == QuadMeshType::MESH_KITE)
        {
            ConformalMappingOneStep(&quadMeshes[a]);
        }
    }
}

void ConformalMapping::ConformalMappingOneStep(QuadMesh *qMesh)
{
    std::vector<std::vector<PlusSignVertex>> tempVertices = qMesh->_plusSignVertices;
    int meshWidth = qMesh->_plusSignVertices.size();
    int meshHeight = qMesh->_plusSignVertices[0].size();

    for(int a = 0; a < meshWidth; a++)
    {
        for(int b = 0; b < meshHeight; b++)
        {
            if(!tempVertices[a][b].shouldMove)
                { continue; }

            AVector curPos = tempVertices[a][b].position;
            AVector sumPositions(0, 0);
            float sumArmLengths = 0;
            float sumArmAngles = 0;
            int numNeighbor = 0;

            // have left and right
            if(a > 0 && a < meshWidth - 1)
            {
                PlusSignVertex lVertex = tempVertices[a - 1][b];
                PlusSignVertex rVertex = tempVertices[a + 1][b];

                // left
                sumPositions += lVertex.position;
                sumArmLengths += curPos.Distance(lVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(-1, 0), (lVertex.position - curPos).Norm());


                // right
                sumPositions += rVertex.position;
                sumArmLengths += curPos.Distance(rVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(1, 0), (rVertex.position - curPos).Norm());

                numNeighbor += 2;
            }
            // have left only
            else if(a > 0)
            {
                PlusSignVertex lVertex = tempVertices[a - 1][b];
                AVector fakeNeighbor = lVertex.position + UtilityFunctions::Rotate( AVector(1, 0) * lVertex.armLength, lVertex.angle);
                sumPositions += fakeNeighbor;
                sumArmLengths += curPos.Distance(lVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(-1, 0), (lVertex.position - curPos).Norm());

                numNeighbor++;
            }
            // have right only
            else if(a < meshWidth - 1)
            {
                PlusSignVertex rVertex = tempVertices[a + 1][b];
                AVector fakeNeighbor = rVertex.position + UtilityFunctions::Rotate( AVector(-1, 0) * rVertex.armLength, rVertex.angle);

                sumPositions += fakeNeighbor;
                sumArmLengths += curPos.Distance(rVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(1, 0), (rVertex.position - curPos).Norm());

                numNeighbor++;
            }

            // have up and down
            if(b > 0 && b < meshHeight - 1)
            {
                PlusSignVertex uVertex = tempVertices[a][b - 1];
                PlusSignVertex bVertex = tempVertices[a][b + 1];

                // up
                sumPositions += uVertex.position;
                sumArmLengths += curPos.Distance(uVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(0, -1), (uVertex.position - curPos).Norm());

                // down
                sumPositions += bVertex.position;
                sumArmLengths += curPos.Distance(bVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(0, 1), (bVertex.position - curPos).Norm());

                numNeighbor += 2;
            }
            // have up only
            else if(b > 0)
            {
                PlusSignVertex uVertex = tempVertices[a][b - 1];
                AVector fakeNeighbor = uVertex.position + UtilityFunctions::Rotate( AVector(0, 1) * uVertex.armLength, uVertex.angle);
                sumPositions += fakeNeighbor;
                sumArmLengths += curPos.Distance(uVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(0, -1), (uVertex.position - curPos).Norm());

                numNeighbor++;
            }
            // have down only
            else if(b < meshHeight - 1)
            {
                PlusSignVertex bVertex = tempVertices[a][b + 1];
                AVector fakeNeighbor = bVertex.position + UtilityFunctions::Rotate( AVector(0, -1) * bVertex.armLength, bVertex.angle);
                sumPositions += fakeNeighbor;
                sumArmLengths += curPos.Distance(bVertex.position);
                sumArmAngles += UtilityFunctions::GetRotation(AVector(0, 1), (bVertex.position - curPos).Norm());

                numNeighbor++;
            }

            sumPositions  = sumPositions / (float)numNeighbor;
            sumArmAngles  = sumArmAngles / (float)numNeighbor;
            sumArmLengths = sumArmLengths / (float)numNeighbor;

            tempVertices[a][b].armLength = sumArmLengths;
            tempVertices[a][b].angle = sumArmAngles;

            if(numNeighbor < 4)
            {
                tempVertices[a][b].position = GetClosestPointFromBorders(*qMesh, sumPositions);
            }
            else
            {
                tempVertices[a][b].position = sumPositions;
            }
        }
    }

    float sumDist = 0;
    for(int a = 0; a < meshWidth; a++)
    {
        for(int b = 0; b < meshHeight; b++)
            { sumDist += qMesh->_plusSignVertices[a][b].position.Distance(tempVertices[a][b].position); }
    }
    _iterDist += sumDist;
    qMesh->_plusSignVertices = tempVertices;
}

void ConformalMapping::MappingInterpolation(QuadMesh oriQMesh, QuadMesh *qMesh)
{
    std::vector<std::vector<PlusSignVertex>> tempVertices;

    // implement right kite first
}

AVector ConformalMapping::GetClosestPointFromBorders(QuadMesh qMesh, AVector pt)
{
    AVector closestPt = pt;
    float dist = std::numeric_limits<float>::max();
    std::vector<ALine> borderLines;
    borderLines.push_back(ALine(qMesh._leftStartPt,  qMesh._rightStartPt));
    borderLines.push_back(ALine(qMesh._leftEndPt,    qMesh._rightEndPt));
    borderLines.push_back(ALine(qMesh._leftStartPt,  qMesh._leftEndPt));
    borderLines.push_back(ALine(qMesh._rightStartPt, qMesh._rightEndPt));
    for(uint a = 0; a < borderLines.size(); a++)
    {
        AVector cPt = UtilityFunctions::GetClosestPoint(borderLines[a].GetPointA(), borderLines[a].GetPointB(), pt);
        if(pt.Distance(cPt) < dist)
        {
            dist = pt.Distance(cPt);
            closestPt = cPt;
        }
    }
    return closestPt;
}

