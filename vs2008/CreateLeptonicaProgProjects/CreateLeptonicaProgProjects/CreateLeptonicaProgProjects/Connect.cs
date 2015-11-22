using System;
using System.IO;
using Extensibility;
using EnvDTE;
using EnvDTE80;
using Microsoft.VisualStudio.CommandBars;
using Microsoft.VisualBasic.FileIO;
using System.Resources;
using System.Reflection;
using System.Globalization;
using System.Diagnostics;
using System.Windows.Forms;

namespace CreateLeptonicaProgProjects
	{
	/// <summary>The object for implementing an Add-in.</summary>
	/// <seealso class='IDTExtensibility2' />
	public class Connect : IDTExtensibility2, IDTCommandTarget
		{
		private DTE2 _applicationObject;
		private AddIn _addInInstance;
	
		/// <summary>Implements the constructor for the Add-in object. Place your initialization code within this method.</summary>
		public Connect()
			{
			}

		/// <summary>Implements the OnConnection method of the IDTExtensibility2 interface. Receives notification that the Add-in is being loaded.</summary>
		/// <param term='application'>Root object of the host application.</param>
		/// <param term='connectMode'>Describes how the Add-in is being loaded.</param>
		/// <param term='addInInst'>Object representing this Add-in.</param>
		/// <seealso class='IDTExtensibility2' />
		public void OnConnection(object application, ext_ConnectMode connectMode, object addInInst, ref Array custom)
			{
			_applicationObject = (DTE2)application;
			_addInInstance = (AddIn)addInInst;

			if (connectMode == ext_ConnectMode.ext_cm_UISetup)
				{
				object []contextGUIDS = new object[] { };
				Commands2 commands = (Commands2)_applicationObject.Commands;

				//DumpCommandBars ();

				//Place the command on the Item context menu.
				Microsoft.VisualStudio.CommandBars.CommandBar itemCommandBar =
					((Microsoft.VisualStudio.CommandBars.CommandBars)_applicationObject.CommandBars)["Item"];

				//This try/catch block can be duplicated if you wish to add multiple commands to be handled by your Add-in,
				//  just make sure you also update the QueryStatus/Exec method to include the new command names.
				try
					{
					//Add a command to the Commands collection:
					Command command = commands.AddNamedCommand2(
						_addInInstance,
						"CreateLeptonicaProgProjects",
						"CreateLeptonicaProgProjects",
						"Executes the command for CreateLeptonicaProgProjects",
						true,
						18,
						ref contextGUIDS,
						(int)vsCommandStatus.vsCommandStatusSupported+(int)vsCommandStatus.vsCommandStatusEnabled,
						(int)vsCommandStyle.vsCommandStylePictAndText,
						vsCommandControlType.vsCommandControlTypeButton
						);

					//Add a control for the command to the Item context menu:
					if((command != null))
						{
						CommandBarControl newCommand = (CommandBarControl) command.AddControl(itemCommandBar,
															itemCommandBar.Controls.Count + 1);
						if (newCommand != null)
							{
							newCommand.Caption = "Create &Project for Leptonica Prog program";
							newCommand.TooltipText = "Create Project for Leptonica Prog program";
							}
						}
					}
				catch(System.ArgumentException)
					{
					//If we are here, then the exception is probably because a command with that name
					//  already exists. If so there is no need to recreate the command and we can 
                    //  safely ignore the exception.
					}
				}
			}

		/// <summary>Implements the OnDisconnection method of the IDTExtensibility2 interface. Receives notification that the Add-in is being unloaded.</summary>
		/// <param term='disconnectMode'>Describes how the Add-in is being unloaded.</param>
		/// <param term='custom'>Array of parameters that are host application specific.</param>
		/// <seealso class='IDTExtensibility2' />
		public void OnDisconnection(ext_DisconnectMode disconnectMode, ref Array custom)
			{
			}

		/// <summary>Implements the OnAddInsUpdate method of the IDTExtensibility2 interface. Receives notification when the collection of Add-ins has changed.</summary>
		/// <param term='custom'>Array of parameters that are host application specific.</param>
		/// <seealso class='IDTExtensibility2' />		
		public void OnAddInsUpdate(ref Array custom)
			{
			}

		/// <summary>Implements the OnStartupComplete method of the IDTExtensibility2 interface. Receives notification that the host application has completed loading.</summary>
		/// <param term='custom'>Array of parameters that are host application specific.</param>
		/// <seealso class='IDTExtensibility2' />
		public void OnStartupComplete(ref Array custom)
			{
			}

		/// <summary>Implements the OnBeginShutdown method of the IDTExtensibility2 interface. Receives notification that the host application is being unloaded.</summary>
		/// <param term='custom'>Array of parameters that are host application specific.</param>
		/// <seealso class='IDTExtensibility2' />
		public void OnBeginShutdown(ref Array custom)
			{
			}
		
		/// <summary>Implements the QueryStatus method of the IDTCommandTarget interface. This is called when the command's availability is updated</summary>
		/// <param term='commandName'>The name of the command to determine state for.</param>
		/// <param term='neededText'>Text that is needed for the command.</param>
		/// <param term='status'>The state of the command in the user interface.</param>
		/// <param term='commandText'>Text requested by the neededText parameter.</param>
		/// <seealso class='Exec' />
		public void QueryStatus(string commandName, vsCommandStatusTextWanted neededText,
								ref vsCommandStatus status, ref object commandText)
			{
			if(neededText == vsCommandStatusTextWanted.vsCommandStatusTextWantedNone)
				{
				if (System.IO.Path.GetFileNameWithoutExtension (_applicationObject.Solution.FileName).ToLower () !=
					"leptonica")
					{
					status = vsCommandStatus.vsCommandStatusInvisible;
					return;
					}

				if (commandName == "CreateLeptonicaProgProjects.Connect.CreateLeptonicaProgProjects")
					{
					UIHierarchy UIH = _applicationObject.ToolWindows.SolutionExplorer;
					System.Array sItems = (System.Array) UIH.SelectedItems;

					//Make sure all selected items are in "prog_files" folder
					foreach (UIHierarchyItem sItem in sItems)
						{
						string topLevelFolderName = GetTopLevelFolderName (sItem);
						if (topLevelFolderName != "prog_files")
							{
							status = vsCommandStatus.vsCommandStatusInvisible;
							return;
							}
						}
		
					status = (vsCommandStatus) vsCommandStatus.vsCommandStatusSupported |
											   vsCommandStatus.vsCommandStatusEnabled;
					return;
					}
				}
			}

		/// <summary>Implements the Exec method of the IDTCommandTarget interface. This is called when the command is invoked.</summary>
		/// <param term='commandName'>The name of the command to execute.</param>
		/// <param term='executeOption'>Describes how the command should be run.</param>
		/// <param term='varIn'>Parameters passed from the caller to the command handler.</param>
		/// <param term='varOut'>Parameters passed from the command handler to the caller.</param>
		/// <param term='handled'>Informs the caller if the command was handled or not.</param>
		/// <seealso class='Exec' />
		public void Exec(string commandName, vsCommandExecOption executeOption,
						 ref object varIn, ref object varOut, ref bool handled)
			{
			handled = false;

			if(executeOption == vsCommandExecOption.vsCommandExecOptionDoDefault)
				{
				if (commandName == "CreateLeptonicaProgProjects.Connect.CreateLeptonicaProgProjects")
					{
					UIHierarchy UIH = _applicationObject.ToolWindows.SolutionExplorer;
					System.Array sItems = (System.Array) UIH.SelectedItems;
					SolutionFolder progProjectsFolder = GetProgProjectsSF();

					//Create projects for all selected items (will be in "prog_files")
					foreach (UIHierarchyItem sItem in sItems)
						{
						string vcprojFile = CreateProgProject(sItem);

						if (vcprojFile != null)
							progProjectsFolder.AddFromFile(vcprojFile);
						}

					handled = true;
					return;
					}
				}
			}

		private void DumpCommandBars ()
			{
			Microsoft.VisualStudio.CommandBars.CommandBars commandBars = 
				(Microsoft.VisualStudio.CommandBars.CommandBars)_applicationObject.CommandBars;
			foreach (Microsoft.VisualStudio.CommandBars.CommandBar cBar in commandBars)
				{
				Debug.Print(cBar.Name);
				}
			}

		private string GetTopLevelFolderName(UIHierarchyItem item)
			{
	        UIHierarchyItems folder = item.Collection;
			UIHierarchyItems previousFolder = null;

			while (((UIHierarchyItem)folder.Parent).Name.ToLower () != "leptonica")
				{
				previousFolder = folder;
				folder = ((UIHierarchyItem) folder.Parent).Collection;
				}

			return ((UIHierarchyItem) previousFolder.Parent).Name.ToLower();
			}

		private SolutionFolder GetProgProjectsSF()
			{
			foreach (Project curProject in _applicationObject.Solution.Projects)
				{
				if (curProject.Name.ToLower () == "prog_projects")
					{
					return (SolutionFolder) curProject.Object;
					}
				}

			return null;
			}

		private string CreateProgProject(UIHierarchyItem item)
			{
			string projectName = System.IO.Path.GetFileNameWithoutExtension(item.Name);
			string progProjectsPath = System.IO.Path.Combine(
										System.IO.Path.GetDirectoryName(_applicationObject.Solution.FullName),
																		"prog_projects");
			string projectDir = System.IO.Path.Combine(progProjectsPath, projectName);

			if (System.IO.Directory.Exists(projectDir))
				{
				string msgStr = projectDir + "\nalready exists. Project not created.";
				System.Windows.Forms.MessageBox.Show(msgStr);
				return null;
				}

			//Copy ioformats_reg directory to new projectName directory
			string templateDir = System.IO.Path.Combine(progProjectsPath, "ioformats_reg");
			Microsoft.VisualBasic.FileIO.FileSystem.CopyDirectory(templateDir, projectDir);

			//Delete any build output directories
			string[] buildDirs = { "LIB Debug", "LIB Release", "DLL Debug", "DLL Release" };
			foreach (string buildDir in buildDirs)
				{
				string dir = System.IO.Path.Combine (projectDir, buildDir);
				if (System.IO.Directory.Exists (dir))
					System.IO.Directory.Delete (dir, true);
				}
	 
			//Rename ioformats_reg.vcproj to projectname.vcproj
			string templateVcprojFile = System.IO.Path.Combine(projectDir, "ioformats_reg.vcproj");
			string vcprojFile = System.IO.Path.Combine(projectDir, projectName + ".vcproj");
			System.IO.File.Move(templateVcprojFile, vcprojFile);
			
			//Fixup new projectname.vcproj
			string fileData = System.IO.File.ReadAllText(vcprojFile);
			fileData = fileData.Replace("ioformats_reg", projectName);
			System.IO.FileAttributes fileAttributes = System.IO.File.GetAttributes(vcprojFile);
			fileAttributes = fileAttributes & ~System.IO.FileAttributes.ReadOnly;
			System.IO.File.SetAttributes(vcprojFile, fileAttributes);
			System.IO.File.WriteAllText(vcprojFile, fileData);

			//Rename ioformats_reg.vcproj.machinename.username.user to projectname.vcproj.machinename.username.user
			string[] userFiles  = 
				System.IO.Directory.GetFiles(projectDir, "*.user", System.IO.SearchOption.TopDirectoryOnly);
			foreach (string userFile in userFiles)
				{
				string newFile = userFile.Replace("ioformats_reg", projectName);
				System.IO.File.Move(userFile, newFile);
				}

			return vcprojFile;
			}
		}
	}
