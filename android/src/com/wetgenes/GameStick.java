package com.wetgenes;

import android.app.NativeActivity;
import android.util.Log;
import android.content.Intent;
import android.content.Context;
import java.io.File;
import java.io.InputStream;

import java.util.*; 

import com.playjam.gamestick.databaseinterfaceservice.*;
import com.playjam.gamestick.downloadservice.*;


public class GameStick  implements DatabaseInterfaceService.IJavaDatabaseInterfaceResponse,DownloadService.IDownloadResponse
{
	public Queue queue;

	public DatabaseInterfaceService database;
	public DownloadService download;

	public GameStick(android.content.Context ctx)
	{
		queue = new LinkedList();
		
		database = new DatabaseInterfaceService(ctx, this);
		download = new DownloadService(ctx, this);
		
		database.LeaderBoard_GetTop50();
	}
   
	
	public String poll()
	{
		return (String) queue.poll();
	}

// dumb escape?
	String esc(String s)
	{
		return s.replace("\\","\\\\").replace("\"","\\\"");
	}

	@Override public void DownloadFailed(String url, String why)
	{
//		Log.d("PLAYJAM", "Failed : " + url + " value " + why);

		StringBuilder s = new StringBuilder();
		
		s.append("{\n");
		s.append("cmd=\"DownloadFailed\",\n");
		s.append("url=\""+esc(url)+"\",\n");
		s.append("why=\""+esc(why)+"\",\n");
		s.append("}\n");
		
		queue.add(s.toString());
	}

	@Override public void DownloadProgress(String url, int progress) 
	{        
//		Log.d("PLAYJAM", "Progress : " + url + " value " + progress);

		StringBuilder s = new StringBuilder();
		
		s.append("{\n");
		s.append("cmd=\"DownloadProgress\",\n");
		s.append("url=\""+esc(url)+"\",\n");
		s.append("progress="+progress+",\n");
		s.append("}\n");
		
		queue.add(s.toString());
	}

	@Override public void DownloadSuccess(String url, String data) 
	{
//		Log.d("PLAYJAM", "Success : " + url + " value " + data);
		// insert code here to copy the file somewhere safe if needed later

		StringBuilder s = new StringBuilder();
		
		s.append("{\n");
		s.append("cmd=\"DownloadSuccess\",\n");
		s.append("url=\""+esc(url)+"\",\n");
		s.append("data=\""+esc(data)+"\",\n");
		s.append("}\n");
		
		queue.add(s.toString());
	}


	public void LeaderBoard_Response (String cmd,LeaderboardData data)
	{
		StringBuilder s = new StringBuilder();
		
		s.append("{\n");
		s.append("cmd=\""+cmd+"\",\n");

		s.append("data={\n");
		for (Iterator iterator = data.GetEntries().iterator(); iterator.hasNext();) 
		{
		  LeaderboardData.Entry item = (LeaderboardData.Entry) iterator.next();

			s.append("{\n");
			s.append("avatar="+item.AvatarID()+",\n");
			s.append("name=\""+esc(item.Name())+"\",\n");
			s.append("score="+item.Score()+",\n");
			s.append("position="+item.Position()+",\n");
			s.append("},\n");
		}
		s.append("},\n");

		s.append("}\n");
		
		queue.add(s.toString());
	}

//	DownloadService m_download_service;

	@Override public void LeaderBoard_GetTop50_Response (LeaderboardData data)
	{
		LeaderBoard_Response("LeaderBoard_GetTop50_Response",data);
	}

	@Override public void LeaderBoard_GetRange_Response (LeaderboardData data)
	{
		LeaderBoard_Response("LeaderBoard_GetRange_Response",data);
	}

	@Override public void LeaderBoard_GetNearest_Response (LeaderboardData data)
	{
		LeaderBoard_Response("LeaderBoard_GetNearest_Response",data);
   	}

	@Override public void LeaderBoard_SaveScore_Response ()
	{
//		Log.d("GameStick", "Save Score Response");

		StringBuilder s = new StringBuilder();
		
		s.append("{\n");
		s.append("cmd=\"LeaderBoard_SaveScore_Response\",\n");
		s.append("}\n");
		
		queue.add(s.toString());
	}

	@Override public void Game_SaveState_Response ()
	{
//		Log.d("GameStick", "Saving");

		StringBuilder s = new StringBuilder();
		
		s.append("{\n");
		s.append("cmd=\"Game_SaveState_Response\",\n");
		s.append("}\n");
		
		queue.add(s.toString());
	}

	@Override public void Game_LoadState_Response (InputStream data)
	{
//		Log.d("GameStick", "Loading data");
		
		StringBuilder s = new StringBuilder();
		
		s.append("{\n");
		s.append("cmd=\"Game_LoadState_Response\",\n");
//		s.append("data=\""+esc(data.toString())+"\"\n");
		s.append("}\n");
		
		queue.add(s.toString());
	}
	
	@Override public void Achievement_GetAllAchievements_Response (Achievements data)
	{
		for (Iterator iterator = data.GetItems().iterator(); iterator.hasNext();) 
		{
			Achievements.Item item = (Achievements.Item) iterator.next();

//			Log.d("GameStick", "ID :" + Integer.toString( item.ID() ));
//			Log.d("GameStick", "Name : " + item.Name());
//			Log.d("GameStick", "Description : " + item.Description());
//			Log.d("GameStick", "Type : " + item.Type());
//			Log.d("GameStick", "fileName : " + item.FileName());
//			Log.d("GameStick", "file URL : " + item.FileURL());
//			Log.d("GameStick", "Completed : " + Boolean.toString( item.isCurrentUserOwner() ));
//			Log.d("GameStick", "XP Value : " + Integer.toString( item.XPValue() ));
		}
	}

	@Override public void Achievement_SetAchievementComplete_Response ()
	{
//		Log.d("GameStick", "Set achievement complete");

		StringBuilder s = new StringBuilder();
		
		s.append("{\n");
		s.append("cmd=\"Achievement_SetAchievementComplete_Response\",\n");
		s.append("}\n");
		
		queue.add(s.toString());
	}

	@Override public void Analytics_GameEvent_Response ()
	{
//		Log.d("GameStick", "Analytics response");

		StringBuilder s = new StringBuilder();
		
		s.append("{\n");
		s.append("cmd=\"Analytics_GameEvent_Response\",\n");
		s.append("}\n");
		
		queue.add(s.toString());
	}

	@Override public void InAppPurchase_GetPurchasedItems_Response (AppItems data)
	{
		for (Iterator iterator = data.GetItems().iterator(); iterator.hasNext();) 
		{
			AppItems.AppItem item = (AppItems.AppItem) iterator.next();
//			Log.d("GameStick", item.toString());
		}
	}

	@Override public void InAppPurchase_GetItemsForPurchase_Response (AppItems data)
	{
		for (Iterator iterator = data.GetItems().iterator(); iterator.hasNext();) 
		{
			AppItems.AppItem item = (AppItems.AppItem) iterator.next();
//			Log.d("GameStick", item.toString());
		}
	}

	@Override public void InAppPurchase_PurchaseItem_Response (boolean bought)
	{
//		Log.d("GameStick", "Purchase response : " + bought);
   	}

	@Override public void InAppPurchase_GetPurchasedItemURL_Response (String url)
	{
//		Log.d("GameStick", "Purchased item url : " + url);
	}

	@Override public void DatabaseRequestFailed (DatabaseInterfaceService.Request id, String message)
	{
//		Log.d("GameStick", "Error : " + message);
	}
	
	
// sending
	
	public void SendScore(int n)
	{
		database.LeaderBoard_SaveScore(n);
	}

	public void RangeScore(int na,int nb)
	{
//		database.LeaderBoard_GetRange(na,nb);
		database.LeaderBoard_GetTop50();
	}

}
