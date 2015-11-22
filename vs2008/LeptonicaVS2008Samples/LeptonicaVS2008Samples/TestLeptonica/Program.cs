using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

//using LeptonicaCLR;

namespace TestLeptonica
	{
	class Program
		{
		static int Main (string[] args)
			{
			if (args.Length != 2)
				{
				Console.WriteLine("Usage: TestLeptonica binImage threshold");
				return 1;
				}

			String binaryImageFile = args[0];
			string threshholdStr = args[1];

			if (!File.Exists (binaryImageFile))
				{
				Console.WriteLine (Path.GetFullPath (binaryImageFile) + " doesn't exist.");
				return 1;
				}

			int bitDepth = LeptonicaCLR.Utils.GetBitDepth (binaryImageFile);
			if (bitDepth != 1)
				{
				Console.WriteLine ("Usage: TestLeptonica binImage threshold\n" +
								   "       TestLeptonica only works with binary images.");
				return 1;
				}
	
			int threshold = Convert.ToInt32(threshholdStr);
			if (threshold < 0 || threshold > 255)
				{
				Console.WriteLine ("Usage: TestLeptonica binImage threshold\n" +
								   "       threshold must be between 0 and 255.");
				return 1;
				}

			String deskewedName = Path.GetFileNameWithoutExtension (binaryImageFile) + "-deskewed-" +
								   threshholdStr + ".tif";
			String diffName = Path.GetFileNameWithoutExtension (deskewedName) + "-diff" +
							  ".tif";

			if (LeptonicaCLR.Utils.DeskewBinaryImage (binaryImageFile, deskewedName, threshold))
				LeptonicaCLR.Utils.CreateBinaryImageDiff (binaryImageFile, deskewedName, diffName,
														  "Threshold " + threshholdStr + " ");

			return 0;
			}
		}
	}
