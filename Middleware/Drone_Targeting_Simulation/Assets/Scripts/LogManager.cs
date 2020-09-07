using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

public class LogManager : MonoBehaviour
{
    public static LogManager Instance { get; private set; }

    public string logFileName = "Log.txt";
    public bool useTabs = true;
    [TextArea]
    public string TestComments = "Moving drone around in circles";

    public TargetingSystem ADLTS;

    private string pathToLogs;
    [System.NonSerialized]
    public char tab;


    private void Awake()
    {
        //This will only pass once at the beggining of the game 
        if (Instance == null)
        {
            //Self reference
            Instance = this;
            //Make this object persistent
            DontDestroyOnLoad(gameObject);
        }
        else
        {
            //Destroy duplicate instance created by changing Scene
            Destroy(gameObject);
        }

        tab = useTabs ? '\t' : ',';
    }

    void Start()
    {
        pathToLogs = Application.dataPath + "/Logs/";
    }

    public void LogParameters(string fileName = "", string Obj = "ADLTS")
    {
        if (fileName == "")
        {
            fileName = logFileName;
        }
        switch (Obj)
        {
            case "ADLTS":
                LogResults(fileName, "-----New Entry: " + System.DateTime.Now + "-----\n\n");

                LogResults(fileName, "******************* Parameters *******************\n");
                LogResults(fileName, "predictVelocity: " + ADLTS.predictVelocity + "\n");
                LogResults(fileName, "noiseDampening: " + ADLTS.noiseDampening + "\n");
                LogResults(fileName, "MaxNoise: " + ADLTS.MaxNoise + "\n");
                LogResults(fileName, "chanceToLoseFrame: " + ADLTS.chanceToLoseFrame + "\n");
                LogResults(fileName, "velocityMaxDegreeChange: " + ADLTS.velocityMaxDegreeChange + "\n\n\n");

                break;
            default:
                Debug.LogError("Unkown Object '" + Obj + "'");
                break;
        }

    }
    public void LogResults(string fileName = "Log.txt", string data = "\n")
    {
        string path = pathToLogs + fileName;

        if (!File.Exists(path))
        {
            File.WriteAllText(path, "Time Created: " + System.DateTime.Now + "\n");
        }

        File.AppendAllText(path, data);
    }
}
