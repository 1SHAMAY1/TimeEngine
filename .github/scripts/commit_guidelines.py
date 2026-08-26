#!/usr/bin/env python3
"""
TimeEngine CI - Commit & PR Guidelines Validator
Validates commit headers against labeler-aligned module tags, proper English phrasing,
strict length limit (< 50 characters), and PR description structure.
"""

import sys
import os
import re
import subprocess
import argparse
from typing import List, Tuple, Set
from report_utils import ReportGenerator

VALID_TAGS: Set[str] = {
    "Core", "Renderer", "Platform", "Window", "Utils", "ECS", "GameFrameWork",
    "Input", "Threading", "Time", "Asset", "Collision", "Events", "Physics",
    "Plugin", "Project", "Scene", "Editor", "Docs", "CI/CD", "Vendor",
    "AI Agents", "Automation", "MAC", "Linux", "Windows", "Release", "Build"
}

LAZY_MESSAGES = {
    "fix", "fixes", "fixed", "update", "updates", "updated", "wip", "work in progress",
    "test", "testing", "asdasd", "asdf", "changes", "stuff", "done", "edit", "edits",
    "misc", "temp", "tmp", "commit", "commit message", "clean", "cleanup"
}

def validate_commit_message(commit_hash: str, commit_msg: str, report: ReportGenerator):
    lines = commit_msg.strip().splitlines()
    if not lines or not lines[0].strip():
        report.add_issue(
            file_path=f"Commit: {commit_hash[:8]}",
            line_number=1,
            rule_id="COMMIT-EMPTY",
            message="Commit message header is completely empty.",
            snippet=""
        )
        return

    header = lines[0].strip()

    # Skip Git/GitHub automated merge and revert commits
    if header.startswith("Merge ") or header.startswith("Revert "):
        return

    # Rule 1: Header Length strictly < 50 characters
    if len(header) >= 50:
        report.add_issue(
            file_path=f"Commit: {commit_hash[:8]}",
            line_number=1,
            rule_id="COMMIT-LEN-LIMIT",
            message=f"Commit header length ({len(header)} chars) exceeds maximum limit of < 50 chars.",
            snippet=header
        )

    # Rule 2: Check for lazy shorthand messages
    clean_lower = header.lower().strip(" .!?:-")
    if clean_lower in LAZY_MESSAGES:
        report.add_issue(
            file_path=f"Commit: {commit_hash[:8]}",
            line_number=1,
            rule_id="COMMIT-LAZY-MSG",
            message=f"Lazy/meaningless commit message detected ('{header}'). Please provide descriptive context.",
            snippet=header
        )

    # Rule 3: Check Tag format: <Tag>: <Description>
    match = re.match(r"^([^:]+):\s*(.+)$", header)
    if not match:
        report.add_issue(
            file_path=f"Commit: {commit_hash[:8]}",
            line_number=1,
            rule_id="COMMIT-FORMAT",
            message="Commit header must follow the format '<Tag>: <Description>' (e.g., 'Docs: Updated camera guide' or 'MAC: Build fixes').",
            snippet=header
        )
    else:
        raw_tag = match.group(1).strip()
        description = match.group(2).strip()

        matched_tag = None
        for valid in VALID_TAGS:
            if raw_tag.lower() == valid.lower():
                matched_tag = valid
                break

        if not matched_tag:
            report.add_issue(
                file_path=f"Commit: {commit_hash[:8]}",
                line_number=1,
                rule_id="COMMIT-TAG-INVALID",
                message=f"Tag '{raw_tag}' is not recognized. Valid tags: {', '.join(sorted(VALID_TAGS))}",
                snippet=header
            )
        elif raw_tag != matched_tag:
            report.add_issue(
                file_path=f"Commit: {commit_hash[:8]}",
                line_number=1,
                rule_id="COMMIT-TAG-CASING",
                message=f"Tag '{raw_tag}' should match canonical casing '{matched_tag}'.",
                snippet=header
            )

        if description and not description[0].isupper():
            report.add_issue(
                file_path=f"Commit: {commit_hash[:8]}",
                line_number=1,
                rule_id="COMMIT-DESC-CASE",
                message="Commit description after '<Tag>:' must start with a capital letter (proper English).",
                snippet=header
            )

        if description.endswith("."):
            report.add_issue(
                file_path=f"Commit: {commit_hash[:8]}",
                line_number=1,
                rule_id="COMMIT-TRAILING-PERIOD",
                message="Commit header should not end with a period.",
                snippet=header
            )

    if len(lines) > 1:
        if lines[1].strip() != "":
            report.add_issue(
                file_path=f"Commit: {commit_hash[:8]}",
                line_number=2,
                rule_id="COMMIT-BODY-SEP",
                message="A blank line is required between the commit header and the commit body.",
                snippet=lines[1]
            )

def validate_pr_metadata(pr_title: str, pr_body: str, report: ReportGenerator):
    if not pr_title:
        return

    if len(pr_title) >= 72:
        report.add_issue(
            file_path="Pull Request Metadata",
            line_number=0,
            rule_id="PR-TITLE-LEN",
            message=f"PR title length ({len(pr_title)} chars) exceeds recommended limit of 72 chars.",
            snippet=pr_title
        )

    match = re.match(r"^([^:]+):\s*(.+)$", pr_title)
    if not match:
        report.add_issue(
            file_path="Pull Request Metadata",
            line_number=0,
            rule_id="PR-TITLE-FORMAT",
            message="PR title must follow '<Tag>: <Description>' format (e.g. 'Renderer: Optimized batch draw pipeline').",
            snippet=pr_title
        )

    if not pr_body or len(pr_body.strip()) < 15:
        report.add_issue(
            file_path="Pull Request Metadata",
            line_number=0,
            rule_id="PR-BODY-EMPTY",
            message="PR body is empty or too short. Please describe the motivation and changes included in this PR.",
            snippet=pr_body or ""
        )

def get_commits_in_range(base_ref: str, head_ref: str) -> List[Tuple[str, str]]:
    try:
        cmd = ["git", "log", "--no-merges", f"{base_ref}..{head_ref}", "--pretty=format:%H%x00%B%x01"]
        out = subprocess.check_output(cmd, encoding="utf-8", errors="replace")
        commits = []
        raw_items = out.split("\x01")
        for item in raw_items:
            item = item.strip()
            if not item:
                continue
            parts = item.split("\x00", 1)
            if len(parts) == 2:
                commits.append((parts[0].strip(), parts[1].strip()))
        return commits
    except Exception as e:
        print(f"[WARN] Failed to read git commit range {base_ref}..{head_ref}: {e}", file=sys.stderr)
        try:
            cmd = ["git", "log", "-1", "--pretty=format:%H%x00%B%x01"]
            out = subprocess.check_output(cmd, encoding="utf-8", errors="replace").strip("\x01").strip()
            if out:
                parts = out.split("\x00", 1)
                return [(parts[0].strip(), parts[1].strip())]
        except Exception:
            pass
        return []

def main():
    parser = argparse.ArgumentParser(description="TimeEngine CI Commit & PR Guidelines Validator")
    parser.add_argument("--base-ref", default="", help="Git base ref or commit")
    parser.add_argument("--head-ref", default="HEAD", help="Git head ref")
    parser.add_argument("--commit-msg", default="", help="Direct commit message to test")
    parser.add_argument("--pr-title", default="", help="Pull Request Title")
    parser.add_argument("--pr-body", default="", help="Pull Request Body")
    parser.add_argument("--causer-email", default="", help="Email of author/causer")
    parser.add_argument("--trigger-context", default="Push / PR", help="Context of run")
    parser.add_argument("--report-dir", default=".github/reports", help="Directory to save reports")
    args = parser.parse_args()

    report = ReportGenerator(
        title="Commit & PR Guidelines Audit",
        description="Audits commit messages and PR metadata for proper English formatting, module tags, and strict header length limits (< 50 chars)."
    )

    if args.commit_msg:
        validate_commit_message("CURRENT", args.commit_msg, report)
    else:
        base = args.base_ref if args.base_ref else "HEAD~1"
        commits = get_commits_in_range(base, args.head_ref)
        if not commits:
            try:
                msg = subprocess.check_output(["git", "log", "-1", "--pretty=%B"], encoding="utf-8", errors="replace")
                commit_hash = subprocess.check_output(["git", "log", "-1", "--pretty=%H"], encoding="utf-8", errors="replace").strip()
                commits = [(commit_hash, msg)]
            except Exception:
                pass

        for c_hash, c_msg in commits:
            validate_commit_message(c_hash, c_msg, report)

    if args.pr_title or args.pr_body:
        validate_pr_metadata(args.pr_title, args.pr_body, report)

    report.export_reports(
        output_dir=args.report_dir,
        base_name="commit_guidelines_report",
        causer_email=args.causer_email,
        trigger_context=args.trigger_context
    )

    if report.has_issues:
        print(f"\n[FAILURE] Commit Guidelines Validator found {report.total_count} issue(s).", file=sys.stderr)
        sys.exit(1)
    else:
        print("\n[SUCCESS] Commit Guidelines Validator passed cleanly with 0 issues.")
        sys.exit(0)

if __name__ == "__main__":
    main()
