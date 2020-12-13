[![StackOverflow](https://stackexchange.com/users/flair/7322082.png)](https://stackoverflow.com/users/5577765/rabbid76?tab=profile)

---

# Ambient Occlusion

## SSAO Screen Space Ambient Occlusion 

**Reference**

Efficient Screen-Space Approach to High-Quality Multi-Scale Ambient Occlusion, 2012; Thai-Duong Hoang, Kok-Lim Low<br/>
[https://www.comp.nus.edu.sg/~lowkl/publications/mssao_visual_computer_2012.pdf]

Fast Precomputed Ambient Occlusion for Proximity Shadows; Mattias Malmer, Fredrik Malmer, Ulf Assarsson, Nicolas Holzschuch<br/>
[http://www.cse.chalmers.se/~uffe/Fast%20Precomputed%20Ambient%20Occlusion%20for%20Proximity-RR-5779.pdf]

Load-Balanced Multi-GPU Ambient Occlusion for Direct Volume Rendering; A. Ancel, J.-M. Dischler, C. Mongenet<br/>
[https://dpt-info.u-strasbg.fr/~dischler/papers/ADM12.pdf]

Scalable Ambient Obscurance, 2012; Morgan McGuire, Michael Mara, David Luebke; NVIDIA<br/>
[http://research.nvidia.com/sites/default/files/pubs/2012-06_Scalable-Ambient-Obscurance/McGuire12SAO.pdf]

Multi-view Ambient Occlusion with Importance Sampling; Kostas Vardis, Georgios Papaioannou, Athanasios Gaitatzes<br/>
[http://graphics.cs.aueb.gr/graphics/docs/papers/MultiviewAmbientOcclusion.pdf]

Screen-Space Ambient Occlusion Using A-buffer Techniques, 2013; Fabian Bauer, Martin Knuth, Arjan Kuijper, Jan Bender<br/>
[https://animation.rwth-aachen.de/media/papers/2013-CADGraphics-MultilayerAO.pdf]

Multi-View Ambient Occlusion for Enhancing Visualization of Raw Scanning Data; Manuele Sabbadin, Gianpaolo Palma, Paolo Cignoni, Roberto Scopigno<br/>
[http://vcg.isti.cnr.it/Publications/2016/SPCS16/final.pdf]

Multi-scale Visualization of Molecular Architecture Using Real-Time Ambient Occlusion in Sculptor; Manuel Wahle, Willy Wriggers<br/>
[https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4623981/]<br/>
[https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4623981/pdf/pcbi.1004516.pdf]

Screen Space Ambient Occlusion; NVIDIA 2008; Louis Bavoil, Miguel Sainz<br/>
[http://artis.inrialpes.fr/Membres/Olivier.Hoel/ssao/nVidiaHSAO/ScreenSpaceAO.pdf]

Multi-Layer Dual-Resolution Screen-Space Ambient Occlusion; Louis Bavoil, Miguel Sainz<br/>
[http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.214.8434&rep=rep1&type=pdf]

VAO++; Practical Volumetric Ambient Occlusion for Games, 2017; J. Bok?ansky, A. Pospisil, J. Bittner<br/>
[http://dcgi.fel.cvut.cz/home/bittner/publications/egsr2017.pdf]

multiresolution ambient occlusion<br/>
[http://www.iquilezles.org/www/articles/multiresaocc/multiresaocc.htm]

[Ambient Occlusion Lighting]<br/>
https://www.cgl.ucsf.edu/chimera/data/ambient-jul2014/ambient.html

Hardware Accelerated Ambient Occlusion Techniques on GPUs; Perumaal Shanmugam, Okan Arikan<br/>
[http://www.students.science.uu.nl/~3220516/advancedgraphics/papers/ssao_predecessor.pdf]

Ambient Occlusion Fields; Janne Kontkanen Samuli Laine<br/>
[https://users.aalto.fi/~laines9/publications/kontkanen2005i3d_paper.pdf]

<br><hr>

## HBAO Horizon Based Ambient Occlusion

Image-Space Horizon-Based Ambient Occlusion; NVIDIA Corporation; Louis Bavoil, Miguel Sainz, Rouslan Dimitrov<br/>
[http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.577.2286&rep=rep1&type=pdf]

Horizon-Based Ambient Occlusion using Compute Shader; Nvidia 2008; Louis Bavoil<br/>
[https://pdfs.semanticscholar.org/66ad/8d9cfb7ba0c239d5586b6a45f1e0fbf5375a.pdf]

An alternative implementation for HBAO<br/>
[http://www.derschmale.com/2013/12/20/an-alternative-implementation-for-hbao-2/]

<br><hr>

## Related

**Reference**

A Comparative Study of Screen-Space Ambient Occlusion Methods<br/>
[http://frederikaalund.com/a-comparative-study-of-screen-space-ambient-occlusion-methods/]

<br><hr>

## TODO

### Dynamically Multi Frequency ambient occlusion

4 channels for 4 different frequencies.

1st channel contains the occlusion calculation to the 8th neighbour fragments.

The 1st sample result for each following channel (frequency) is the result of the previous channel (frequency).

The samples are distributed to 3 ring sections (1st is technically a circle).
The sample points are linear distributed on the occlusion radius in ascending order. Each sample is added to its corresponding frequency channel (2 to 4).


<br/><hr/>

<a href="https://stackexchange.com/users/7322082/rabbid76"><img src="https://stackexchange.com/users/flair/7322082.png" width="208" height="58" alt="profile for Rabbid76 on Stack Exchange, a network of free, community-driven Q&amp;A sites" title="profile for Rabbid76 on Stack Exchange, a network of free, community-driven Q&amp;A sites" /></a>
